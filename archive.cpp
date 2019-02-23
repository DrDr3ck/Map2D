#include "archive.h"

#include "character.h"
#include "logger.h"
#include "translator.h"
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <algorithm>

namespace { // anonymous namespace

bool isTag(const std::string& str, const std::string& tag_origin, bool is_end=false) {
    std::string tag("<");
    if( is_end ) {
        tag.append("/");
    }
    tag.append(tag_origin);
    std::size_t tag_found = str.find(tag);
    return tag_found != std::string::npos;
}

bool startsWithTag(const std::string& str) {
    std::string trimmed_str = Utility::trim(str);
    if( trimmed_str.size() >= 2 && trimmed_str[0] == '<' && trimmed_str[1] != '/' ) {
        return true;
    }
    return false;
}

bool isEndTag(const std::string& str, const std::string& tag_origin) {
    return isTag(str, tag_origin, true);
}

// example of random tag: <counted_item nb="2" attr="value">copper_cable</counted_item>
//                        <TagName TagAttrName="TagAttrValue">TagValue</TagName>

std::string getTagName(const std::string& str) {
    // <TagName TagAttrName="TagAttrValue">TagValue</TagName>
    // get end tag
    std::string tag("</");
    std::string endtag(">");
    std::size_t tag_idx = str.find(tag);
    if( tag_idx != std::string::npos ) {
        std::size_t end_tag_idx = str.find(endtag, tag_idx);
        std::string tag_name = str.substr(tag_idx+2, end_tag_idx-tag_idx-2);
        return tag_name;
    }
    return "";
}

std::string getTagValue(const std::string& str) {
    // <TagName TagAttrName="TagAttrValue">TagValue</TagName>
    // find value between > and </
    std::string endtag(">");
    std::size_t end_tag_idx = str.find(endtag);
    if( end_tag_idx != std::string::npos ) {
        std::string tag("</");
        std::size_t tag_idx = str.find(tag, end_tag_idx);
        std::string tag_value = str.substr(end_tag_idx+1, tag_idx-end_tag_idx-1);
        return tag_value;
    }
    return "";
}

int getTagAttrCount(const std::string& str) {
    int n = int(std::count(str.begin(), str.end(), '='));
    return n;
}

std::string getTagAttrName(const std::string& str, int attr_occurrence) {
    // find string '="'
    int index = 0;
    std::size_t equal_idx = str.find("=\"");
    if( equal_idx == std::string::npos ) {
        return "";
    }
    while( index != attr_occurrence ) { // need to find the attr_occurrence of '="'
        equal_idx = str.find("=\"");
        if( equal_idx == std::string::npos ) {
            return "";
        }
        index++;
    }
    if( equal_idx != std::string::npos ) {
        std::size_t attr_name_idx = str.rfind(" ", equal_idx);
        std::string attr_name = str.substr(attr_name_idx+1, equal_idx-attr_name_idx-1);
        return attr_name;
    }
    return "";
}

std::string getTagAttrValue(const std::string& str, int attr_occurrence) {
    // find string '="'
    int index = 0;
    std::size_t equal_idx = str.find("=\"");
    if( equal_idx == std::string::npos ) {
        return "";
    }
    while( index != attr_occurrence ) { // need to find the attr_occurrence of '="'
        equal_idx = str.find("=\"");
        if( equal_idx == std::string::npos ) {
            return "";
        }
        index++;
    }
    if( equal_idx != std::string::npos ) {
        std::size_t attr_value_idx = str.find("\"", equal_idx+2);
        std::string attr_value = str.substr(equal_idx+2, attr_value_idx-equal_idx-2);
        return attr_value;
    }
    return "";
}

std::string getAttribute(const std::string& str, const std::string& attr_origin) {
    std::string attr(attr_origin);
    attr.append("=");
    std::size_t attr_found = str.find(attr);
    if( attr_found != std::string::npos ) {
        attr_found+=attr.size()+1;
        std::size_t end_found = str.find("\"", attr_found+1);
        return str.substr(attr_found,end_found-attr_found);
    }
    return "";
}

} // anonymous namespace

/********************************************************************/

void ArchiveConverter::load(GameBoard* board, const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        Logger::error() << tr("unable to open file for load: ") << filename << Logger::endl;
        return;
    }

    DataConverter* converter = nullptr;
    std::string str;
    while (std::getline(file, str)) {
        if( isTag(str, "mapdata") ) {
            converter = new MapDataConverter(board->data());
        }
        if( isEndTag(str, "mapdata") ) {
            delete converter;
            converter = nullptr;
        }

        if( isTag(str, "group") ) {
            converter = new CharacterConverter(board->group());
        }
        if( isEndTag(str, "group") ) {
            delete converter;
            converter = nullptr;
        }

        if( isTag(str, "position_objects") ) {
            converter = new ObjectConverter(board->data());
        }
        if( isEndTag(str, "position_objects") ) {
            delete converter;
            converter = nullptr;
        }
        if( converter != nullptr ) {
            converter->load(str);
        }
    }

    file.close();
}

void ArchiveConverter::save(GameBoard* board, const std::string& filename) {
    std::ofstream file(filename);
    if (!file) {
        Logger::error() << tr("unable to open file for save: ") << filename << Logger::endl;
        return;
    }

    Logger::info() << tr("Saving ") << filename << "..." << Logger::endl;

    MapDataConverter mconverter(board->data());
    mconverter.save(file);

    CharacterConverter cconverter(board->group());
    cconverter.save(file);

    ObjectConverter oconverter(board->data());
    oconverter.save(file);

    file.close();
}

/********************************************************************/

void MapDataConverter::load(const std::string& str) {
    if( isTag(str, "mapdata") ) {
        std::string width_str = getAttribute(str, "width");
        int width = atoi(width_str.c_str());
        std::string height_str = getAttribute(str, "height");
        int height = atoi(height_str.c_str());
        data_->reset(width,height);
        std::string map_name = getAttribute(str, "image");
        data_->setMapImageName(map_name);
    }
    if( !inTile_ ) {
        if( isTag(str, "tile") ) {
            std::string x_str = getAttribute(str, "x");
            x_ = atoi(x_str.c_str());
            std::string y_str = getAttribute(str, "y");
            y_ = atoi(y_str.c_str());
            inTile_ = true;
            occurrences_ = 0;
            items_.clear();
        }
    }
    if( inTile_ ) {
        if( isTag(str, "id") ) {
            std::string value_str = getAttribute(str, "value");
            tile_id_ = atoi(value_str.c_str());
        }
        if( isTag(str, "type") ) {
            std::string value_str = getAttribute(str, "value");
            tile_type_ = stringTileToType(value_str);
        }
        if( isTag(str, "occurrence") ) {
            std::string value_str = getAttribute(str, "value");
            occurrences_ = atoi(value_str.c_str());
        }
        if( isTag(str, "counteditem") ) {
            std::string item_name = getAttribute(str, "name");
            std::string value_str = getAttribute(str, "value");
            int item_count = atoi(value_str.c_str());
            items_.push_back( std::pair<std::string, int>(item_name, item_count));
        }
        if( isTag(str, "btype") ) {
            std::string value_str = getAttribute(str, "value");
            tile_btype_ = stringTileToBType(value_str);
        }
        if( isTag(str, "ftype") ) {
            std::string value_str = getAttribute(str, "value");
            tile_ftype_ = stringTileToFType(value_str);
        }
        if( isEndTag(str, "tile") ) {
            Tile& cur_tile = data_->tile(x_,y_);
            cur_tile.setTile(tile_id_, tile_type_, tile_btype_, tile_ftype_);
            cur_tile.setOccurrences(occurrences_);
            for( auto item : items_ ) {
                std::string item_name = item.first;
                int item_count = item.second;
                if( item_name != "none" && item_count > 0 ) {
                    cur_tile.addItem(BasicItem(item_name), item_count);
                }
            }
            inTile_ = false;
        }
    }
}

std::string MapDataConverter::typeTileToString(Tile::Type type) const {
    if( type == Tile::BLOCK ) return "BLOCK";
    if( type == Tile::DOOR ) return "DOOR";
    if( type == Tile::FLOOR ) return "FLOOR";
    if( type == Tile::WALL ) return "WALL";
    if( type == Tile::EMPTY ) return "EMPTY";
    Logger::error() << "unable to find string for type: " << type << Logger::endl;
    return "NONE";
}

std::string MapDataConverter::btypeTileToString(Tile::BType type) const {
    if( type == Tile::NONE ) return "NONE";
    if( type == Tile::WATER ) return "WATER";
    if( type == Tile::SAND ) return "SAND";
    if( type == Tile::DIRT ) return "DIRT";
    if( type == Tile::GRASS ) return "GRASS";
    if( type == Tile::COAL ) return "COAL";
    if( type == Tile::ROCK ) return "ROCK";
    if( type == Tile::COPPER ) return "COPPER";
    if( type == Tile::IRON ) return "IRON";
    Logger::error() << "unable to find string for background type: " << type << Logger::endl;
    return "NONE";
}

std::string MapDataConverter::ftypeTileToString(Tile::FType type) const {
    if( type == Tile::METAL ) return "METAL";
    if( type == Tile::PLASTIC ) return "PLASTIC";
    Logger::error() << "unable to find string for floor type: " << type << Logger::endl;
    return "METAL";
}

Tile::Type MapDataConverter::stringTileToType(const std::string& str) const {
    if( str == "BLOCK" ) return Tile::BLOCK;
    if( str == "DOOR" ) return Tile::DOOR;
    if( str == "FLOOR" ) return Tile::FLOOR;
    if( str == "WALL" ) return Tile::WALL;
    if( str == "EMPTY" ) return Tile::EMPTY;
    Logger::error() << "unable to find type for string: " << str << Logger::endl;
    return Tile::BLOCK;
}

Tile::BType MapDataConverter::stringTileToBType(const std::string& str) const {
    if( str == "NONE" ) return Tile::NONE;
    if( str == "WATER" ) return Tile::WATER;
    if( str == "SAND" ) return Tile::SAND;
    if( str == "COPPER" ) return Tile::COPPER;
    if( str == "IRON" ) return Tile::IRON;
    if( str == "DIRT" ) return Tile::DIRT;
    if( str == "COAL" ) return Tile::COAL;
    if( str == "GRASS" ) return Tile::GRASS;
    if( str == "ROCK" ) return Tile::ROCK;
    Logger::error() << "unable to find background type for string: " << str << Logger::endl;
    return Tile::NONE;
}

Tile::FType MapDataConverter::stringTileToFType(const std::string& str) const {
    if( str == "METAL" ) return Tile::METAL;
    if( str == "PLASTIC" ) return Tile::PLASTIC;
    Logger::error() << "unable to find floor type for string: " << str << Logger::endl;
    return Tile::METAL;
}

void MapDataConverter::save(std::ofstream& file) {
    file << "<mapdata width=\"" << data_->width() << "\" height=\"" << data_->height() << "\" image=\"" << data_->getMapImageName() << "\">" << std::endl;
    for( int j = 0 ; j < data_->height(); j++ ) {
        for( int i = 0 ; i < data_->width(); i++ ) {
            const Tile& cur = data_->tile(i,j);
            file << "  <tile x=\"" << i << "\" y=\"" << j << "\">" << std::endl;
            file << "    <id value=\"" << cur.id() << "\" />" << std::endl;
            file << "    <type value=\"" << typeTileToString(cur.cell_type()) << "\" />" << std::endl;
            file << "    <occurrence value=\"" << Utility::itos(cur.occurrences()) << "\" />" << std::endl;
            file << "    <btype value=\"" << btypeTileToString(cur.background_type()) << "\" />" << std::endl;
            file << "    <ftype value=\"" << ftypeTileToString(cur.floor_type()) << "\" />" << std::endl;
            for( auto counted_item : cur.counted_items() ) {
                if( !counted_item.isNull() ) {
                    file << "    <counteditem name=\"" << counted_item.item().name() << "\" value=\"" << counted_item.count() << "\" />" << std::endl;
                }
            }
            file << "  </tile>" << std::endl;
        }
    }
    file << "</mapdata>" << std::endl;
}

/********************************************************************/

void CharacterConverter::load(const std::string& str) {
    if( !inPeople ) {
        if( isTag(str, "people") ) {
            name = getAttribute(str, "name");
            carried_items.clear();
            inPeople = true;
        }
    }

    if( inPeople ) {
        if( isTag(str, "direction") ) {
            std::string x_str = getAttribute(str, "x");
            int x = atoi(x_str.c_str());
            std::string y_str = getAttribute(str, "y");
            int y = atoi(y_str.c_str());
            dir = {x,y};
        }
        if( isTag(str, "position") ) {
            std::string x_str = getAttribute(str, "x");
            int x = atoi(x_str.c_str());
            std::string y_str = getAttribute(str, "y");
            int y = atoi(y_str.c_str());
            pos = {x,y};
        }
        if( isTag(str, "image_id") ) {
            std::string str_value = getAttribute(str, "value");
            image_id = atoi(str_value.c_str());
        }
        if( isTag(str, "activity") ) {
            std::string str_value = getAttribute(str, "value");
            activity_percentage = atoi(str_value.c_str());
        }
        if( isTag(str, "carried_item") ) {
            std::string str_value = getAttribute(str, "name");
            carried_items.push_back(BasicItem(str_value));
        }

        if( isEndTag(str, "people") ) {
            Character* people = new Character(name, pos, image_id);
            people->setDirection(dir.x, dir.y);
            for( auto carried_item : carried_items ) {
                people->carryItem(carried_item);
            }
            group_->add(people);
            inPeople = false;
        }
    }
}

void CharacterConverter::save(std::ofstream& file) {
    file << "<group>" << std::endl;
    for( auto people : group_->group() ) {
        file << "  <people name=\"" << people->name() << "\">" << std::endl;
        Position pos = people->tilePosition();
        file << "    <position x=\"" << pos.x << "\" y=\"" << pos.y << "\" />" << std::endl;
        Direction dir = people->direction();
        file << "    <direction x=\"" << dir.x << "\" y=\"" << dir.y << "\" />" << std::endl;
        file << "    <activity value=\"" << people->activityPercent() << "\" />" << std::endl;
        file << "    <image_id value=\"" << people->imageIdForArchive() << "\" />" << std::endl;
        const std::vector<BasicItem>& carried_items = people->carriedItems();
        for( unsigned int i=0; i < carried_items.size(); i++ ) {
            const BasicItem& item = carried_items[i];
            file << "    <carried_item>" << item.name() << "</carried_item>" << std::endl;
        }
        file << "  </people>" << std::endl;
    }

    file << "</group>" << std::endl;
}

/********************************************************************/

void ObjectConverter::load(const std::string& str) {
    if( !inObject ) {
        if( isTag(str, "object") ) {
            name = getAttribute(str, "name");
            inObject = true;
            return;
        }
    }

    if( inObject ) {
        if( isTag(str, "position") ) {
            std::string x_str = getAttribute(str, "x");
            int x = atoi(x_str.c_str());
            std::string y_str = getAttribute(str, "y");
            int y = atoi(y_str.c_str());
            pos = {x,y};
        } else if( startsWithTag(str) ) {
            // get tag name, tag value and tag attributes !!
            std::string tag_name = getTagName(str);
            std::string tag_value = getTagValue(str);
            int nb_attr = getTagAttrCount(str);
            std::vector<Attr> attrs;
            for( int i=0; i < nb_attr; i++ ) {
                std::string attr_name = getTagAttrName(str, i);
                std::string attr_value = getTagAttrValue(str, i);
                attrs.push_back(Attr({attr_name, attr_value}));
            }
            if( tag_name != "" && tag_value != "" ) {
                NameValue nv{tag_name, tag_value};
                nodes.push_back(Node({nv, attrs}));
            }
        }
    }
    if( isEndTag(str, "object") ) {
        Object* object = nullptr;
        if( name == "chest" ) {
            object = new Chest(8);
        } else if( name == "workbench" ) {
            object = new WorkBench();
        } else if( name == "breaker" ) {
            object = new Breaker();
        } else if( name == "stone_furnace" ) {
            object = new StoneFurnace();
        } else if( name == "command_center" ) {
            object = new CommandCenter();
        } else if( name == "electric_furnace" ) {
            object = new ElectricFurnace();
        }
        if( object != nullptr ) {
            data_->addObject(object, pos.x, pos.y);
            // add also attributes !!
            for( auto node : nodes ) {
                NameValue nv = node.first;
                std::vector<Attr> attrs = node.second;
                object->setNode(nv.first, attrs, nv.second);
            }
        }
        inObject = false;
        nodes.clear();
    }
}

void ObjectConverter::save_nodes(std::ofstream& file, Object* object) {
    // file << "    <counted_item nb=\"" << citem.count() << "\">" << citem.item().name() << "</counted_item>" << std::endl;
    for(int i=0; i < object->getNodeCount(); i++ ) {
        const std::string name = object->getNodeName(i); // counted_item
        file << "    <" << name;
        for( int j=0; j < object->getAttributeCount(i); j++ ) {
            const std::string attr_name = object->getAttributeName(i,j); // nb
            const std::string attr_value = object->getAttributeValue(i,j); // citem.count()
            file << " " << attr_name << "=\"" << attr_value << "\"";
        }
        const std::string value = object->getNodeValue(i); // citem.item().name()
        file << ">" << value << "</" << name << ">" << std::endl;
    }
}

void ObjectConverter::save(std::ofstream& file) {
    file << "<position_objects>" << std::endl;
    for( Object* object : data_->objects() ) {
        file << "  <object name=\"" << object->name() << "\">" << std::endl;
        Position position_object = object->tilePosition();
        file << "    <position x=\"" << position_object.x << "\" y=\"" << position_object.y << "\" />" << std::endl;
        // save nodes of object
        save_nodes(file, object);
        file << "  </object>" << std::endl;
    }
    file << "</position_objects>" << std::endl;
}

