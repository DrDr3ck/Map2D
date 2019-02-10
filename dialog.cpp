#include "dialog.h"

#include "sdl_camera.h"
#include "sdl_button.h"
#include "craft_mgr.h"

#include <tuple>

/***********************************/

Dialog::Dialog(int x, int y, int width, int height) : View(), x_(x), y_(y), width_(width), height_(height) {
    surface_ = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
    SDL_Color bgcolor = {211,211,211,255};
    setBackgroundColor(bgcolor);
}

Dialog::~Dialog() {
    SDL_FreeSurface(surface_);
}

SDL_Rect Dialog::getTitleRect(int extension) const {
    SDL_Rect window_rect = {1-extension,1-extension,width_-2+2*extension,18+2*extension};
    return window_rect;
}

SDL_Rect Dialog::getKillRect() const {
    SDL_Rect window_rect = {width_ - 18, 2,16,16};
    return window_rect;
}

SDL_Rect Dialog::getMinimizeRect(int extension) const {
    SDL_Rect window_rect = {width_ - 18 - 20-extension, 2-extension,16+2*extension,16+2*extension};
    return window_rect;
}

SDL_Rect Dialog::getCenterRect() const {
    SDL_Rect window_rect = {width_ - 18 - 20 - 20, 2,16,16};
    return window_rect;
}

void Dialog::setBackgroundColor(const SDL_Color& bgcolor) {
    background_color_ = bgcolor;
    SDL_FillRect(surface_, NULL, SDL_MapRGBA(surface_->format, 0, 0, 0, background_color_.a));
    SDL_Rect rect = {1,20,width_-2,height_-21};
    SDL_FillRect(
        surface_, &rect,
        SDL_MapRGBA(surface_->format, background_color_.r, background_color_.g, background_color_.b, background_color_.a)
    );

    SDL_Rect window_rect = getTitleRect();
    SDL_FillRect(surface_, &window_rect, SDL_MapRGBA(surface_->format, background_color_.r*0.9, background_color_.g*0.9, background_color_.b*0.9, background_color_.a));

    SDL_Rect kill_rect = getKillRect();
    SDL_FillRect(surface_, &kill_rect, SDL_MapRGBA(surface_->format, 255, 100, 100, background_color_.a));

    SDL_Rect minimize_rect = getMinimizeRect();
    SDL_FillRect(surface_, &minimize_rect, SDL_MapRGBA(surface_->format, 100, 100, 255, background_color_.a));
    minimize_rect = getMinimizeRect(-5);
    SDL_FillRect(surface_, &minimize_rect, SDL_MapRGBA(surface_->format, 200, 200, 200, background_color_.a));

    SDL_Rect center_rect = getCenterRect();
    SDL_FillRect(surface_, &center_rect, SDL_MapRGBA(surface_->format, 100, 255, 100, background_color_.a));
}

void Dialog::do_render(Camera* camera, double delay_in_ms) {
    std::ignore = delay_in_ms;
    SDLCamera* sdl_camera = dynamic_cast<SDLCamera*>(camera);
    SDL_Renderer* main_renderer = sdl_camera->main_renderer();
    SDL_Texture* texture = SDL_CreateTextureFromSurface(main_renderer,surface_);
    SDL_Rect rect = {x_, y_, width_, height_ };
    if( minimized_ ) {
        SDL_Rect from = getTitleRect(1);
        rect.w = from.w;
        rect.h = from.h;
        sdl_camera->displayTexture(texture, &rect, &from);
        SDL_SetRenderDrawColor( main_renderer, 50, 50, 50, 255 );
        SDL_RenderDrawRect(main_renderer, &rect);
    } else {
        sdl_camera->displayTexture(texture, &rect);
    }
    SDL_DestroyTexture(texture);
}

/*!
 * \return true if this dialog has the focus
 * (i.e. if mouse cursor is over the dialog).
 */
bool Dialog::hasFocus(int mouse_x, int mouse_y) {
    float width = width_;
    float height = height_;
    if( minimized_ ) {
        width = getTitleRect(1).w;
        height = getTitleRect(1).h;
    }
    if( mouse_x < x_ ) return false;
    if( mouse_x > x_+width ) return false;
    if( mouse_y < y_ ) return false;
    if( mouse_y > y_+height ) return false;
    return true;
}

bool Dialog::handleEvent(Camera* camera) {
    SDLCamera* sdl_camera = dynamic_cast<SDLCamera*>(camera);
    const SDL_Event& event = sdl_camera->event();
    int rel_mouse_x = sdl_camera->mouse_x() - x_;
    int rel_mouse_y = sdl_camera->mouse_y() - y_;
    switch( event.type ) {
        case SDL_MOUSEBUTTONDOWN:
            if( event.button.button == SDL_BUTTON_LEFT ) {
                SDL_Rect kill_rect = getKillRect();
                SDL_Rect window_rect = getTitleRect();
                SDL_Rect minimize_rect = getMinimizeRect();
                SDL_Rect center_rect = getCenterRect();
                if( Utility::contains(kill_rect, rel_mouse_x, rel_mouse_y) ) {
                    // kill this dialog !!
                    kill_me_ = true;
                } else if( Utility::contains(minimize_rect, rel_mouse_x, rel_mouse_y) ) {
                    minimized_ = ! minimized_;
                } else if( Utility::contains(center_rect, rel_mouse_x, rel_mouse_y) ) {
                    // get tile and center it
                    Position position = tilePosition();
                    sdl_camera->mapView()->restoreCenterTile( position );
                } else if( Utility::contains(window_rect, rel_mouse_x, rel_mouse_y) ) {
                    grabbing_ = true;
                    rel_grab_x_ = rel_mouse_x;
                    rel_grab_y_ = rel_mouse_y;
                }
            }
            break;
        case SDL_MOUSEBUTTONUP:
            if( event.button.button == SDL_BUTTON_LEFT ) {
                grabbing_ = false;
            }
            break;
        case SDL_MOUSEMOTION:
            //onMouseMove(event.motion.x, event.motion.y);
            if( grabbing_ ) {
                x_ = x_ + rel_mouse_x - rel_grab_x_;
                y_ = y_ + rel_mouse_y - rel_grab_y_;
            }
            break;
        default:
            return false; // event has not been handled
    }
    return true; // event has been handled
}

/**************************************/

RobotDialog::RobotDialog(Character* robot, int mouse_x, int mouse_y) : Dialog(mouse_x, mouse_y, 250, 200) {
    robot_ = robot;
    SDL_Color bgcolor = {255,211,211,255};
    setBackgroundColor(bgcolor);
}

RobotDialog::~RobotDialog() {
    robot_ = nullptr;
}

Position RobotDialog::tilePosition() const {
    return robot_->tilePosition();
}

void RobotDialog::do_render(Camera* camera, double delay_in_ms) {
    Dialog::do_render(camera, delay_in_ms);
    SDLCamera* sdl_camera = dynamic_cast<SDLCamera*>(camera);

    std::string title_str = tr(" - Robot");
    SDLText title(title_str, "pixel11", 11);
    title.set_position(3+x_,2+y_);
    SDL_Color title_bgcolor = getBackgroundColor();
    title_bgcolor.r = title_bgcolor.r*0.9;
    title_bgcolor.g = title_bgcolor.g*0.9;
    title_bgcolor.b = title_bgcolor.b*0.9;
    title.setBackgroundColor(title_bgcolor);
    sdl_camera->displayText(title, false);

    if( minimized_ ) {
        return;
    }

    std::string str = tr("Name: ");
    str+=robot_->name();
    str+= "\n" + tr("Slot: ") + Utility::itos(robot_->maxCarry()-robot_->maxCarriable()) + "/" + Utility::itos(robot_->maxCarry());
    str+="\n" + robot_->actionDescription();
    SDLText text(str);
    text.set_position(9+x_,25+y_);
    text.setBackgroundColor(getBackgroundColor());
    sdl_camera->displayText(text, false);
}

/**************************************/

ObjectDialog::ObjectDialog(PositionObject pobject, int mouse_x, int mouse_y) : Dialog(mouse_x, mouse_y, 300, 400) {
    pobject_ = pobject;
    SDL_Color bgcolor = {211,211,255,255};
    setBackgroundColor(bgcolor);
}

ObjectDialog::~ObjectDialog() {
}

Position ObjectDialog::tilePosition() const {
    Position position = {pobject_.x, pobject_.y};
    return position;
}

void ObjectDialog::do_render(Camera* camera, double delay_in_ms) {
    Dialog::do_render(camera, delay_in_ms);
    SDLCamera* sdl_camera = dynamic_cast<SDLCamera*>(camera);
    Object* object = pobject_.object;

    std::string title_str(" - ");
    title_str.append( object->userName() );
    SDLText title(title_str, "pixel11", 11);
    title.set_position(3+x_,3+y_);
    SDL_Color title_bgcolor = getBackgroundColor();
    title_bgcolor.r = title_bgcolor.r*0.9;
    title_bgcolor.g = title_bgcolor.g*0.9;
    title_bgcolor.b = title_bgcolor.b*0.9;
    title.setBackgroundColor(title_bgcolor);
    sdl_camera->displayText(title, false);

    if( minimized_ ) {
        return;
    }

    std::string str = tr("Name: ");
    str+=object->userName();
    str+="\n" + tr("Tile: ") + Utility::itos(pobject_.x) + " " + Utility::itos(pobject_.y);

    if( object->hasCrafts() ) {
        // get list of crafts
        std::vector<Craft*> crafts = CraftMgr::instance()->craftsForMachine(object->name());
        for( Craft* craft : crafts) {
            str+="\n" + craft->name() + " (" + Utility::itos(craft->time()) + " sec) with";
            for( CountedItem item : craft->getItems() ) {
                str+="\n * " + item.item().name() + " x " + Utility::itos(item.count());
            }
        }
    } else {
        // get object nodes
        int node_count = object->getNodeCount();
        for( int node_index=0; node_index < node_count; node_index++ ) {
            std::string node_string = object->getNodeString(node_index);
            str+="\n" + tr(node_string);
        }
    }
    SDLText text(str);
    text.set_position(9+x_,25+y_);
    text.setBackgroundColor(getBackgroundColor());
    sdl_camera->displayText(text, false);
}

bool ObjectDialog::buttonClicked(SDLButton* button, Position mouse_position) {
    if( button == nullptr ) return false;
    const SDL_Rect& button_rect = button->rect();
    return Utility::contains(button_rect, mouse_position.x, mouse_position.y);
}

// create dedicated ObjectDialog if needed
// (i.e. SmelterDialog, CrafterDialog, ...)
ObjectDialog* ObjectDialog::createDialog(PositionObject pobject, int x, int y) {
    if( pobject.object == nullptr ) return nullptr;
    std::string object_name = pobject.object->name();
    std::cout << "create object dialog for " << pobject.object->name() << std::endl;
    if( Utility::endsWith(object_name, "furnace") ) {
        return new SmelterDialog(pobject,x,y);
    }
    return new ObjectDialog(pobject,x,y);
}

void ObjectDialog::execute() {
}

/**************************************/

SmelterDialog::SmelterDialog(PositionObject pobject, int mouse_x, int mouse_y) : ObjectDialog(pobject, mouse_x, mouse_y) {

}

SmelterDialog::~SmelterDialog() {
    delete craft_button_;
    delete one_button_;
    delete ten_button_;
}

void SmelterDialog::do_render(Camera* camera, double delay_in_ms) {
    ObjectDialog::do_render(camera, delay_in_ms);
    SDLCamera* sdl_camera = dynamic_cast<SDLCamera*>(camera);

    if( craft_button_ == nullptr ) {
        craft_button_ = new SDLTextButton(sdl_camera, "Smelt", x_, y_);
        one_button_ = new SDLTextButton(sdl_camera, "+1", x_, y_);
        ten_button_ = new SDLTextButton(sdl_camera, "+10", x_, y_);
    }

    const SDL_Rect& craft_rect = craft_button_->rect();
    craft_button_->setPosition(x_+width_-5-craft_rect.w, y_+height_-5-craft_rect.h);
    sdl_camera->displayButton(craft_button_);

    const SDL_Rect& one_rect = one_button_->rect();
    one_button_->setPosition(x_+5, y_+height_-5-craft_rect.h);
    sdl_camera->displayButton(one_button_);
    ten_button_->setPosition(x_+5+one_rect.w+5, y_+height_-5-craft_rect.h);
    sdl_camera->displayButton(ten_button_);
}

bool SmelterDialog::handleEvent(Camera* camera) {
    if( craft_button_ == nullptr ) return false;
    SDLCamera* sdl_camera = dynamic_cast<SDLCamera*>(camera);
    const SDL_Event& event = sdl_camera->event();
    //int rel_mouse_x = sdl_camera->mouse_x() - x_;
    //int rel_mouse_y = sdl_camera->mouse_y() - y_;
    Position mouse_position = {sdl_camera->mouse_x(), sdl_camera->mouse_y()};
    switch( event.type ) {
        case SDL_MOUSEBUTTONDOWN:
            if( event.button.button == SDL_BUTTON_LEFT ) {
                if( buttonClicked(craft_button_, mouse_position) ) {
                    execute();
                }
                if( buttonClicked(one_button_, mouse_position) ) {
                    std::cout << "+1" << std::endl;
                }
                if( buttonClicked(ten_button_, mouse_position) ) {
                    std::cout << "+10" << std::endl;
                }
            }
    }
    return Dialog::handleEvent(camera);
}

void SmelterDialog::execute() {
}

/**************************************/
