#include "archive.h"

#include "character.h"
#include "logger.h"
#include <stdlib.h>
#include <iostream>
#include <fstream>

namespace { // anonymous namespace

//bool startsWith(const std::string& str, const std::string& prefix) {
//    return str.rfind(prefix, 0) == 0;
//}

bool isTag(const std::string& str, const std::string& tag_origin, bool is_end=false) {
    std::string tag("<");
    if( is_end ) {
        tag.append("/");
    }
    tag.append(tag_origin);
    std::size_t tag_found = str.find(tag);
    return tag_found != std::string::npos;
}

bool isEndTag(const std::string& str, const std::string& tag_origin) {
    return isTag(str, tag_origin, true);
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
        Logger::error() << "unable to open file for load: " << filename << Logger::endl;
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
        if( converter != nullptr ) {
            converter->load(str);
        }
    }

    file.close();
}

void ArchiveConverter::save(GameBoard* board, const std::string& filename) {
    std::ofstream file(filename);
    if (!file) {
        Logger::error() << "unable to open file for save: " << filename << Logger::endl;
        return;
    }

    Logger::info() << "Saving " << filename << "..." << Logger::endl;

    MapDataConverter mconverter(board->data());
    mconverter.save(file);

    CharacterConverter cconverter(board->group());
    cconverter.save(file);

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
    }
    if( !inTile_ ) {
        if( isTag(str, "tile") ) {
            std::string x_str = getAttribute(str, "x");
            x_ = atoi(x_str.c_str());
            std::string y_str = getAttribute(str, "y");
            y_ = atoi(y_str.c_str());
            inTile_ = true;
            occurrences_ = 0;
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
        if( isTag(str, "occurrences") ) {
            std::string value_str = getAttribute(str, "value");
            occurrences_ = atoi(value_str.c_str());
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
    if( type == Tile::DIRT ) return "DIRT";
    if( type == Tile::GRASS ) return "GRASS";
    if( type == Tile::DIRT ) return "DIRT";
    if( type == Tile::ROCK ) return "ROCK";
    Logger::error() << "unable to find string for background type: " << type << Logger::endl;
    return "NONE";
}

std::string MapDataConverter::ftypeTileToString(Tile::FType type) const {
    if( type == Tile::METAL ) return "METAL";
    if( type == Tile::PLASTIC ) return "PLASTIC";
    Logger::error() << "unable to find string for background type: " << type << Logger::endl;
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
    if( str == "DIRT" ) return Tile::DIRT;
    if( str == "GRASS" ) return Tile::GRASS;
    if( str == "DIRT" ) return Tile::DIRT;
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
    file << "<mapdata width=\"" << data_->width() << "\" height=\"" << data_->height() << "\">" << std::endl;
    for( int j = 0 ; j < data_->height(); j++ ) {
        for( int i = 0 ; i < data_->width(); i++ ) {
            const Tile& cur = data_->tile(i,j);
            file << "  <tile x=\"" << i << "\" y=\"" << j << "\">" << std::endl;
            file << "    <id value=\"" << cur.id() << "\" />" << std::endl;
            file << "    <type value=\"" << typeTileToString(cur.cell_type()) << "\" />" << std::endl;
            file << "    <occurrence value=\"" << Utility::itos(cur.occurrences()) << "\" />" << std::endl;
            file << "    <btype value=\"" << btypeTileToString(cur.background_type()) << "\" />" << std::endl;
            file << "    <ftype value=\"" << ftypeTileToString(cur.floor_type()) << "\" />" << std::endl;
            file << "  </tile>" << std::endl;
        }
    }
    file << "</mapdata>" << std::endl;
}

void CharacterConverter::load(const std::string& str) {
    if( !inPeople ) {
        if( isTag(str, "people") ) {
            name = getAttribute(str, "name");
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

        if( isEndTag(str, "people") ) {
            Character* people = new Character(name, pos, image_id);
            people->setDirection(dir.x, dir.y);
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
        file << "  </people>" << std::endl;
    }

    file << "</group>" << std::endl;
}

