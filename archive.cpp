#include "archive.h"
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
        std::cout << "unable to open file for load: " << filename << "\n";
        return;
    }

    DataConverter* converter = nullptr;
    std::string str;
    while (std::getline(file, str)) {
        if( isTag(str, "mapdata") ) {
            converter = new MapDataConverter(board->data());
        }
        if( isTag(str, "mapdata", true) ) {
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
        std::cout << "unable to open file for save: " << filename << std::endl;
        return;
    }

    MapDataConverter converter(board->data());
    converter.save(file);

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
    if( !inTile ) {
        if( isTag(str, "tile") ) {
            std::string x_str = getAttribute(str, "x");
            x = atoi(x_str.c_str());
            std::string y_str = getAttribute(str, "y");
            y = atoi(y_str.c_str());
            inTile = true;
        }
    }
    if( inTile ) {
        if( isTag(str, "id") ) {
            std::string value_str = getAttribute(str, "value");
            tile_id = atoi(value_str.c_str());
        }
        if( isTag(str, "type") ) {
            std::string value_str = getAttribute(str, "value");
            tile_type = stringTileToType(value_str);
        }
        if( isTag(str, "btype") ) {
            std::string value_str = getAttribute(str, "value");
            tile_btype = stringTileToBType(value_str);
        }
        if( isTag(str, "ftype") ) {
            std::string value_str = getAttribute(str, "value");
            tile_ftype = stringTileToFType(value_str);
        }
        if( isEndTag(str, "tile") ) {
            Tile& cur_tile = data_->tile(x,y);
            cur_tile.setTile(tile_id, tile_type, tile_btype, tile_ftype);
            inTile = false;
        }
    }
}

std::string MapDataConverter::typeTileToString(Tile::Type type) const {
    if( type == Tile::BLOCK ) return "BLOCK";
    if( type == Tile::DOOR ) return "DOOR";
    if( type == Tile::FLOOR ) return "FLOOR";
    if( type == Tile::WALL ) return "WALL";
    if( type == Tile::EMPTY ) return "EMPTY";
    std::cout << "unable to find string for type: " << type << std::endl;
    return "NONE";
}

std::string MapDataConverter::btypeTileToString(Tile::BType type) const {
    if( type == Tile::NONE ) return "NONE";
    if( type == Tile::GRASS ) return "GRASS";
    if( type == Tile::EARTH ) return "EARTH";
    if( type == Tile::ROCK ) return "ROCK";
    std::cout << "unable to find string for background type: " << type << std::endl;
    return "NONE";
}

std::string MapDataConverter::ftypeTileToString(Tile::FType type) const {
    if( type == Tile::METAL ) return "METAL";
    if( type == Tile::PLASTIC ) return "PLASTIC";
    std::cout << "unable to find string for background type: " << type << std::endl;
    return "METAL";
}

Tile::Type MapDataConverter::stringTileToType(const std::string& str) const {
    if( str == "BLOCK" ) return Tile::BLOCK;
    if( str == "DOOR" ) return Tile::DOOR;
    if( str == "FLOOR" ) return Tile::FLOOR;
    if( str == "WALL" ) return Tile::WALL;
    if( str == "EMPTY" ) return Tile::EMPTY;
    std::cout << "unable to find type for string: " << str << std::endl;
    return Tile::BLOCK;
}

Tile::BType MapDataConverter::stringTileToBType(const std::string& str) const {
    if( str == "NONE" ) return Tile::NONE;
    if( str == "GRASS" ) return Tile::GRASS;
    if( str == "EARTH" ) return Tile::EARTH;
    if( str == "ROCK" ) return Tile::ROCK;
    std::cout << "unable to find background type for string: " << str << std::endl;
    return Tile::NONE;
}

Tile::FType MapDataConverter::stringTileToFType(const std::string& str) const {
    if( str == "METAL" ) return Tile::METAL;
    if( str == "PLASTIC" ) return Tile::PLASTIC;
    std::cout << "unable to find floor type for string: " << str << std::endl;
    return Tile::METAL;
}

void MapDataConverter::save(std::ofstream& file) {
    file << "<mapdata width=\"" << data_->width() << "\" height=\"" << data_->height() << "\">" << std::endl;
    for( int j = 0 ; j < data_->height(); j++ ) {
        for( int i = 0 ; i < data_->width(); i++ ) {
            const Tile& cur = data_->tile(i,j);
            file << "  <tile x=\"" << i << "\" y=\"" << j << "\">" << std::endl;
            file << "    <id value=\"" << cur.id() << "\" />" << std::endl;
            file << "    <type value=\"" << typeTileToString(cur.type()) << "\" />" << std::endl;
            file << "    <btype value=\"" << btypeTileToString(cur.background_type()) << "\" />" << std::endl;
            file << "    <ftype value=\"" << ftypeTileToString(cur.floor_type()) << "\" />" << std::endl;
            file << "  </tile>" << std::endl;
        }
    }
    file << "</mapdata>" << std::endl;
}
