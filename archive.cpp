#include "archive.h"
#include <stdlib.h>
#include <iostream>
#include <fstream>

bool startsWith(const std::string& str, const std::string& prefix) {
    return str.rfind(prefix, 0) == 0;
}

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

void MapDataConverter::load(MapData* data, const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cout << "unable to open file for load: " << filename << "\n";
        return;
    }
    std::string str;
    int x = 0;
    int y = 0;
    int tile_id = 0;
    Tile::Type tile_type = Tile::BLOCK;
    bool inTile = false;
    while (std::getline(file, str)) {
        if( isTag(str, "mapdata") ) {
            std::string width_str = getAttribute(str, "width");
            int width = atoi(width_str.c_str());
            std::string height_str = getAttribute(str, "height");
            int height = atoi(height_str.c_str());
            data->reset(width,height);
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
            if( isEndTag(str, "tile") ) {
                Tile& cur_tile = data->tile(x,y);
                cur_tile.setTile(tile_id, tile_type);
                inTile = false;
            }
        }
    }

    //for( int i=0; i < width_; i++ ) {
    //    for( int j=0; j < height_; j++ ) {
    //        Tile& cur = tile(i,j);
    //    }
    //}

    file.close();
}

std::string MapDataConverter::typeTileToString(Tile::Type type) const {
    if( type == Tile::BLOCK ) return "BLOCK";
    if( type == Tile::DOOR ) return "DOOR";
    if( type == Tile::EMPTY ) return "EMPTY";
    std::cout << "unable to find string for type: " << type << std::endl;
    return "NONE";
}

Tile::Type MapDataConverter::stringTileToType(const std::string& str) const {
    if( str == "BLOCK" ) return Tile::BLOCK;
    if( str == "DOOR" ) return Tile::DOOR;
    if( str == "EMPTY" ) return Tile::EMPTY;
    std::cout << "unable to find type for string: " << str << std::endl;
    return Tile::BLOCK;
}

void MapDataConverter::save(MapData* data, const std::string& filename) {
    std::ofstream file(filename);
    if (!file) {
        std::cout << "unable to open file for save: " << filename << std::endl;
        return;
    }

    file << "<mapdata width=\"" << data->width() << "\" height=\"" << data->height() << "\">" << std::endl;
    for( int j = 0 ; j < data->height(); j++ ) {
        for( int i = 0 ; i < data->width(); i++ ) {
            const Tile& cur = data->tile(i,j);
            file << "  <tile x=\"" << i << "\" y=\"" << j << "\">" << std::endl;
            file << "    <id value=\"" << cur.id() << "\" />" << std::endl;
            file << "    <type value=\"" << typeTileToString(cur.type()) << "\" />" << std::endl;
            file << "  </tile>" << std::endl;
        }
    }
    file << "</mapdata>" << std::endl;

    file.close();
}
