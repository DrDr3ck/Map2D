#include "dialog.h"

#include "sdl_camera.h"
#include "sdl_button.h"
#include "craft_mgr.h"
#include "font.h"

#include <tuple>
#include <algorithm>
#include <cmath>

/***********************************/

Dialog::Dialog(int x, int y, int width, int height, bool modal) : View(), x_(x), y_(y), width_(width), height_(height), modal_(modal) {
    surface_ = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
    SDL_Color bgcolor = {211,211,211,255};
    setBackgroundColor(bgcolor);
    // try to put the dialog in the window (no part outside)
    if( x_ + width_ > Camera::cur_camera->width() ) {
        x_ = Camera::cur_camera->width() - width_;
    }
    if( y_ + height_ > Camera::cur_camera->height() ) {
        y_ = Camera::cur_camera->height() - height_;
    }
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

    if( !modal_ ) {
        SDL_Rect minimize_rect = getMinimizeRect();
        SDL_FillRect(surface_, &minimize_rect, SDL_MapRGBA(surface_->format, 100, 100, 255, background_color_.a));
        minimize_rect = getMinimizeRect(-5);
        SDL_FillRect(surface_, &minimize_rect, SDL_MapRGBA(surface_->format, 200, 200, 200, background_color_.a));

        SDL_Rect center_rect = getCenterRect();
        SDL_FillRect(surface_, &center_rect, SDL_MapRGBA(surface_->format, 100, 255, 100, background_color_.a));
    }
}

void Dialog::drawTitle(SDLCamera* sdl_camera, const std::string& title_str) {
    SDLText title(title_str, FontLib::fontFamily(), FontLib::fontTitleSize());
    title.set_position(3+x_,3+y_);
    SDL_Color title_bgcolor = getBackgroundColor();
    title_bgcolor.r = title_bgcolor.r*0.9;
    title_bgcolor.g = title_bgcolor.g*0.9;
    title_bgcolor.b = title_bgcolor.b*0.9;
    title.setBackgroundColor(title_bgcolor);
    sdl_camera->displayText(title, false, true);
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
    if( !grabbing_ && (rel_mouse_x < 0 || rel_mouse_x > width_ || rel_mouse_y < 0 || rel_mouse_y > height_) ) {
        return false; // mouse is not over the dialog
    }
    switch( event.type ) {
        case SDL_MOUSEBUTTONDOWN:
            if( event.button.button == SDL_BUTTON_LEFT ) {
                SDL_Rect kill_rect = getKillRect();
                SDL_Rect title_rect = getTitleRect();
                SDL_Rect minimize_rect = getMinimizeRect();
                SDL_Rect center_rect = getCenterRect();
                if( Utility::contains(kill_rect, rel_mouse_x, rel_mouse_y) ) {
                    // kill this dialog !!
                    kill_me_ = true;
                } else if( !modal_ && Utility::contains(minimize_rect, rel_mouse_x, rel_mouse_y) ) {
                    minimized_ = ! minimized_;
                } else if( !modal_ && Utility::contains(center_rect, rel_mouse_x, rel_mouse_y) ) {
                    // get tile and center it
                    Position position = tilePosition();
                    if( sdl_camera->mapView() != nullptr ) {
                        sdl_camera->mapView()->restoreCenterTile( position );
                    }
                } else if( Utility::contains(title_rect, rel_mouse_x, rel_mouse_y) ) {
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

bool Dialog::buttonClicked(SDLButton* button, Position mouse_position) {
    if( button == nullptr ) return false;
    const SDL_Rect& button_rect = button->rect();
    return Utility::contains(button_rect, mouse_position.x, mouse_position.y);
}

/**************************************/

ModalDialog::ModalDialog(
    const std::string& title,
    int x, int y, int width, int height
) : Dialog(x,y,width,height,true), title_(title) {
}

ModalDialog::~ModalDialog() {
}

void ModalDialog::do_render(Camera* camera, double delay_in_ms) {
    Dialog::do_render(camera, delay_in_ms);
    SDLCamera* sdl_camera = dynamic_cast<SDLCamera*>(camera);

    drawTitle(sdl_camera, title_);
}

bool ModalDialog::handleEvent(Camera* camera) {
    Dialog::handleEvent(camera);
    return true;
}

/**************************************/

RobotDialog::RobotDialog(Character* robot, int mouse_x, int mouse_y) : Dialog(mouse_x, mouse_y, 250, 250) {
    robot_ = robot;
    SDL_Color bgcolor = {255,211,211,255};
    setBackgroundColor(bgcolor);
    const std::vector<BasicItem>& carried_items = robot_->carriedItems();
    for( int i = 0; i < robot_->maxCarry(); i++ ) {
        SDLButton* button = nullptr;
        int x = i%2;
        int y = int(i/2);
        if( i < int(carried_items.size()) ) {
            const BasicItem& item = carried_items.at(i);
            std::string icon_name("items/");
            icon_name.append(item.name());
            icon_name.append("_item.png");
            button = new SDLButton(icon_name, "TBD", 35+100*x, 60+50*y);
        } else {
            button = new SDLButton(35+100*x, 60+50*y);
        }
        button->setTooltipPosition(SDLButton::TooltipPosition::BOTTOM);
        buttons_.push_back( button );
    }
}

RobotDialog::~RobotDialog() {
    robot_ = nullptr;
    buttons_.erase(buttons_.begin(), buttons_.end());
    buttons_.clear();
}

Position RobotDialog::tilePosition() const {
    return robot_->tilePosition();
}

bool RobotDialog::handleEvent(Camera* camera) {
    if( drop_button_ == nullptr ) return false;
    SDLCamera* sdl_camera = dynamic_cast<SDLCamera*>(camera);
    const SDL_Event& event = sdl_camera->event();
    Position mouse_position = {sdl_camera->mouse_x(), sdl_camera->mouse_y()};
    switch( event.type ) {
    case SDL_MOUSEBUTTONDOWN:
        if( event.button.button == SDL_BUTTON_LEFT ) {
            if( buttonClicked(drop_button_, mouse_position) ) {
                robot_->releaseItems();
            }
        }
        break;
    }
    return Dialog::handleEvent(camera);
}

void RobotDialog::do_render(Camera* camera, double delay_in_ms) {
    Dialog::do_render(camera, delay_in_ms);
    SDLCamera* sdl_camera = dynamic_cast<SDLCamera*>(camera);

    std::string title_str = tr(" - Robot");
    drawTitle(sdl_camera, title_str);

    if( minimized_ ) {
        return;
    }

    std::string str = tr("Name: ");
    str+=robot_->name();
    str+= "\n" + tr("Slot: ") + Utility::itos(robot_->maxCarry()-robot_->maxCarriable()) + "/" + Utility::itos(robot_->maxCarry());
    str+="\n" + robot_->actionDescription();
    SDLText text(str, FontLib::fontFamily());
    text.set_position(9+x_,25+y_);
    text.setBackgroundColor(getBackgroundColor());
    sdl_camera->displayText(text, false, true);

    if( drop_button_ == nullptr ) {
        drop_button_ = new SDLTextButton(sdl_camera, tr("drop all"), 9+9, 25);
    }
    drop_button_->setPosition(9+text.rect().w+9+x_, 25+int(text.rect().h/3)+y_);
    sdl_camera->displayButton(drop_button_);

    const std::vector<BasicItem>& carried_items = robot_->carriedItems();
    int maxI = std::min(carried_items.size(),buttons_.size());
    for( int i=0; i < maxI; i++ ) {
        BasicItem item = carried_items.at(i);
        SDLButton* button = buttons_.at(i);
        std::string icon_name("items/");
        icon_name.append(item.name());
        icon_name.append("_item.png");
        button->setIcon(icon_name);
        std::string text = tr(item.name());
        button->setText(text);
        sdl_camera->displayButton(button,x_+5,y_+25);
    }
}

/**************************************/

ObjectDialog::ObjectDialog(Object* object, int mouse_x, int mouse_y) : Dialog(mouse_x, mouse_y, 300, 400) {
    object_ = object;
    SDL_Color bgcolor = {211,211,255,255};
    setBackgroundColor(bgcolor);
}

ObjectDialog::~ObjectDialog() {
}

Position ObjectDialog::tilePosition() const {
    return object_->tilePosition();
}

void ObjectDialog::do_render(Camera* camera, double delay_in_ms) {
    Dialog::do_render(camera, delay_in_ms);
    SDLCamera* sdl_camera = dynamic_cast<SDLCamera*>(camera);

    std::string title_str(" - ");
    title_str.append( object_->userName() );
    drawTitle(sdl_camera, title_str);

    if( minimized_ ) {
        return;
    }

    std::string str = tr("Name: ");
    str+=object_->userName();
    str+="\n" + tr("Tile: ") + Utility::itos(object_->tilePosition().x) + " " + Utility::itos(object_->tilePosition().y);

    // get object nodes
    int node_count = object_->getNodeCount();
    for( int node_index=0; node_index < node_count; node_index++ ) {
        std::string node_string = object_->getNodeString(node_index);
        str+="\n* " + tr(node_string);
    }

    SDLText text(str, FontLib::fontFamily());
    text.set_position(9+x_,25+y_);
    text.setBackgroundColor(getBackgroundColor());
    sdl_camera->displayText(text, false, true);
}

// create dedicated ObjectDialog according to object
// (i.e. SmelterDialog, CrafterDialog, ...)
ObjectDialog* ObjectDialog::createDialog(Object* object, int x, int y) {
    if( object == nullptr ) return nullptr;
    std::string object_name = object->name();
    if( Utility::endsWith(object_name, "furnace") ) {
        return new SmelterDialog(object,x,y);
    } else if( Utility::endsWith(object_name, "chest") ) {
        return new ObjectDialog(object,x,y);
    } else if( Utility::endsWith(object_name, "motor") ) {
        Motor* motor = dynamic_cast<Motor*>(object);
        return new MotorDialog(motor,x,y);
    } else if( object_name == "engine" ) {
        return new ObjectDialog(object,x,y);
    } else if( Utility::endsWith(object_name, "center") ) {
        CommandCenter* cc = CommandCenter::cur_command_center;
        return new CommandCenterDialog(cc,x,y);
    }
    return new CrafterDialog(object,x,y);
}

/**************************************/

MotorDialog::MotorDialog(
    Motor* motor, int mouse_x, int mouse_y
) : ObjectDialog(motor, mouse_x, mouse_y) {
    motor_ = motor;
}

MotorDialog::~MotorDialog() {
}

void MotorDialog::do_render(Camera* camera, double delay_in_ms) {
    Dialog::do_render(camera, delay_in_ms);

    SDLCamera* sdl_camera = dynamic_cast<SDLCamera*>(camera);
    std::string title_str(" - ");
    title_str.append( object_->userName() );
    drawTitle(sdl_camera, title_str);

    if( minimized_ ) {
        return;
    }
}

/**************************************/

CommandCenterDialog::CommandCenterDialog(
    CommandCenter* command_center, int mouse_x, int mouse_y
) : ObjectDialog(command_center, mouse_x, mouse_y) {
    command_center_ = command_center;
    const std::vector<CountedItem>& stored_items = command_center_->storedItems();
    int index = 0;
    for( int j=0; j < 7; j++ ) {
        for( int i=0; i < 3; i++ ) {
            SDLButton* button = nullptr;
            if( index < int(stored_items.size()) ) {
                const CountedItem& item = stored_items.at(index);
                std::string icon_name("items/");
                icon_name.append(item.item().name());
                icon_name.append("_item.png");
                button = new SDLButton(icon_name, "TBD", 35+90*i, 50*j);
            } else {
                button = new SDLButton(35+90*i, 50*j);
            }
            index++;
            button->setTooltipPosition(SDLButton::TooltipPosition::BOTTOM);
            buttons_.push_back( button );
        }
    }
}

CommandCenterDialog::~CommandCenterDialog() {
    buttons_.erase(buttons_.begin(), buttons_.end());
    buttons_.clear();
}

void CommandCenterDialog::do_render(Camera* camera, double delay_in_ms) {
    Dialog::do_render(camera, delay_in_ms);

    SDLCamera* sdl_camera = dynamic_cast<SDLCamera*>(camera);
    std::string title_str(" - ");
    title_str.append( object_->userName() );
    drawTitle(sdl_camera, title_str);

    if( minimized_ ) {
        return;
    }

    const std::vector<CountedItem>& stored_items = command_center_->storedItems();
    int maxI = std::min(stored_items.size(),buttons_.size());
    for( int i=0; i < maxI; i++ ) {
        CountedItem item = stored_items.at(i);
        SDLButton* button = buttons_.at(i);
        std::string icon_name("items/");
        icon_name.append(item.item().name());
        icon_name.append("_item.png");
        button->setIcon(icon_name);
        std::string text(Utility::itos(item.count()));
        button->setText(text);
        sdl_camera->displayButton(button,x_+5,y_+25);
    }
}

/**************************************/

CrafterDialog::CrafterDialog(Object* object, int mouse_x, int mouse_y) : ObjectDialog(object, mouse_x, mouse_y) {
    craft_button_label_ = "Craft";
    for( auto co : object->getCrafts() ) {
        selected_craft_ = co.first;
        addCraft(co.second, false);
    }
    selected_craft_ = nullptr;
}

CrafterDialog::~CrafterDialog() {
    delete craft_button_;
    delete one_button_;
    delete ten_button_;
}

void CrafterDialog::do_render(Camera* camera, double delay_in_ms) {
    Dialog::do_render(camera, delay_in_ms);
    SDLCamera* sdl_camera = dynamic_cast<SDLCamera*>(camera);

    std::string title_str(" - ");
    title_str.append( object_->userName() );
    drawTitle(sdl_camera, title_str);

    if( minimized_ ) {
        return;
    }

    // display the crafted buttons according to object
    bool restore_buttons = false;
    if( craft_buttons_.size() != object()->getCrafts().size() ) {
        restore_buttons = true;
    } else if( craft_buttons_.size() > 0 ) {
        auto co = object()->getCrafts().at(0);
        auto cbo = craft_buttons_.at(0);
        if( co.second != cbo.occurrence ) {
            restore_buttons = true;
        }
    }
    if( restore_buttons ) {
        Craft* craft = selected_craft_;
        craft_buttons_.clear();
        for( auto co : object()->getCrafts() ) {
            selected_craft_ = co.first;
            addCraft(co.second, false);
        }
        selected_craft_ = craft;
    }

    std::string str = tr("Click to get list of ingredient");

    if( recipe_buttons_.size() == 0 ) {
        // get list of crafts
        std::vector<Craft*> crafts = CraftMgr::instance()->craftsForMachine(object_->name());
        for( Craft* craft : crafts) {
            SDLButton* button = new SDLButton(CraftMgr::getPixmapName(craft), x_, y_);
            recipe_buttons_.push_back( std::make_pair(craft, button) );
        }
    }

    int x = x_;
    for( auto recipe_button : recipe_buttons_ ) {
        SDLButton* button = recipe_button.second;
        x += 5;
        button->setPosition(x, 25+y_+5);
        x += button->rect().w;
        sdl_camera->displayButton(button);
        if( recipe_button.first == selected_craft_ ) {
            SDL_SetRenderDrawColor( sdl_camera->main_renderer(), 55, 55, 55, 255 );
            SDL_RenderDrawRect( sdl_camera->main_renderer(), &button->rect() );
        }
    }

    if( selected_craft_ != nullptr ) {
        str = tr(selected_craft_->name()) + " x " + Utility::itos(selected_craft_->occurrence()) + " (" + Utility::itos(selected_craft_->time()) + " sec)";
        str+= "\n" + tr("Ingredients");
        for( CountedItem item : selected_craft_->getItems() ) {
            str+="\n * " + tr(item.item().name()) + " x " + Utility::itos(item.count());
        }
    }
    SDLText text(str, FontLib::fontFamily());
    text.set_position(9+x_,25+y_+50);
    text.setBackgroundColor(getBackgroundColor());
    sdl_camera->displayText(text, false, true);

    if( craft_button_ == nullptr ) {
        craft_button_ = new SDLTextButton(sdl_camera, craft_button_label_, x_, y_);
        craft_button_->setVisible(false);
        SDL_Color button_bgcolor = getBackgroundColor();
        button_bgcolor.r = button_bgcolor.r*0.9;
        button_bgcolor.g = button_bgcolor.g*0.9;
        button_bgcolor.b = button_bgcolor.b*0.9;
        one_button_ = new SDLTextButton(sdl_camera, "+1", x_, y_, SDLText::black(), button_bgcolor);
        ten_button_ = new SDLTextButton(sdl_camera, "+10", x_, y_, SDLText::black(), button_bgcolor);
    }

    const SDL_Rect& craft_rect = craft_button_->rect();
    craft_button_->setPosition(x_+width_-5-craft_rect.w, y_+height_-5-craft_rect.h);
    sdl_camera->displayButton(craft_button_);
    if( craft_button_->isVisible() ) {
        SDL_SetRenderDrawColor( sdl_camera->main_renderer(), 255, 55, 55, 255 );
        SDL_RenderDrawRect( sdl_camera->main_renderer(), &craft_button_->rect() );
    }

    bool display_buttons = (selected_craft_ != nullptr);
    const SDL_Rect& one_rect = one_button_->rect();
    one_button_->setPosition(x_+5, y_+height_-5-craft_rect.h);
    one_button_->setVisible(display_buttons);
    sdl_camera->displayButton(one_button_);
    ten_button_->setPosition(x_+5+one_rect.w+5, y_+height_-5-craft_rect.h);
    ten_button_->setVisible(display_buttons);
    sdl_camera->displayButton(ten_button_);

    x = x_;
    int y = y_+height_-craft_rect.h-5;
    for( auto craft_button : craft_buttons_ ) {
        SDLButton* button = craft_button.button;
        x += 20;
        button->setPosition(x, y-5-button->rect().h);
        x += button->rect().w;
        sdl_camera->displayButton(button);
    }
}

bool CrafterDialog::handleEvent(Camera* camera) {
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
                    addCraft(1);
                }
                if( buttonClicked(ten_button_, mouse_position) ) {
                    addCraft(10);
                }
                for( auto craft_button : recipe_buttons_ ) {
                    SDLButton* button = craft_button.second;
                    if( buttonClicked(button, mouse_position) ) {
                        selectCraft( craft_button.first );
                    }
                }
            } else if( event.button.button == SDL_BUTTON_RIGHT ) {
                int index = 0;
                for( auto craft_button : craft_buttons_ ) {
                    SDLButton* button = craft_button.button;
                    if( buttonClicked(button, mouse_position) ) {
                        // remove item index from Object
                        object()->removeCraft(index);
                        //craft_buttons_.erase( craft_buttons_.begin() + index );
                        break;
                    }
                    index++;
                }
            }
    }
    return Dialog::handleEvent(camera);
}

void CrafterDialog::execute() {
    // move crafts in object and close the dialog
    kill_me_ = true;
}

void CrafterDialog::addCraft(int occ, bool in_object_too) {
    if( selected_craft_ == nullptr ) return;
    if( craft_buttons_.size() == 5 ) {
        Logger::warning() << tr("Cannot add more items in this crafter") << Logger::endl;
        return;
    }
    SDLButton* button = new SDLButton(CraftMgr::getPixmapName(selected_craft_), Utility::itos(occ), x_, y_);
    craft_buttons_.push_back(CraftOccButton{selected_craft_,occ,button});
    if( in_object_too ) {
        object()->addCraft(selected_craft_, occ);
    }
}

void CrafterDialog::selectCraft(Craft* craft) {
    selected_craft_ = craft;
}

/**************************************/

SmelterDialog::SmelterDialog(Object* object, int mouse_x, int mouse_y) : CrafterDialog(object, mouse_x, mouse_y) {
    craft_button_label_ = "Smelt";
}

SmelterDialog::~SmelterDialog() {
}

void SmelterDialog::execute() {
    CrafterDialog::execute();
}

/**************************************/
