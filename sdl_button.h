#ifndef sdl_button_h
#define sdl_button_h

#include "button.h"
#include "sdl_camera.h"

class SDLTool;

/********************************************************************/

class SDLButtonManager : public ButtonManager {
public:
    SDLButtonManager() : ButtonManager() {}
    virtual ~SDLButtonManager() {}

    virtual void do_render(Camera* camera, double delay_in_ms);
    virtual bool handleEvent(Camera* camera) override;
};

/********************************************************************/

class SDLButton : public Button {
public:
    enum class TooltipPosition {BOTTOM, RIGHT, OVER};
public:
    SDLButton(const std::string& icon_name, int x, int y);
    SDLButton(const std::string& icon_name, const std::string& text, int x, int y);
    SDLButton(int x, int y);
    virtual ~SDLButton();

    virtual void activate() override;
    virtual void deactivate() override;

    void setText(const std::string& text) { text_ = text; }
    virtual const std::string& text() const { return text_; }

    void setIcon(const std::string& icon_name);

    TooltipPosition tooltipPosition() const { return tooltip_position_; }
    void setTooltipPosition(TooltipPosition position) { tooltip_position_ = position; }

    virtual void setSize(int w, int h) override;
    virtual void setPosition(int x,int y) override;

    SDL_Texture* getTexture(SDL_Renderer* renderer);
    const SDL_Rect& rect() const { return rect_; }
    void setTexture(SDL_Texture* texture);

protected:
    SDL_Surface* surface_;
    SDL_Texture* texture_;
    SDL_Rect rect_;
    std::string text_;
    TooltipPosition tooltip_position_ = TooltipPosition::RIGHT;
};

class SDLCheckBox : public Button {
    SDLCheckBox(int x, int y);
    virtual ~SDLCheckBox() = default;

    virtual void activate() override;
    virtual void deactivate() override;

protected:
    bool value_;
};

/********************************************************************/

class SDLButtonMenu : public SDLButton {
public:
    SDLButtonMenu(MenuButton* menu, std::string name, int x, int y) : SDLButton(name, x,y), menu_(menu) {
        tooltip_position_ = TooltipPosition::BOTTOM;
    }
    virtual ~SDLButtonMenu() {}

    virtual void activate() override;
    virtual void deactivate() override;
    virtual bool checkButtonPressed(bool button_pressed) const override;
private:
    MenuButton* menu_;
};

/********************************************************************/

class SDLQuitButton : public SDLButton {
public:
    SDLQuitButton(SDLCamera* camera, int x, int y) : SDLButton("buttons/quit.png", tr("Quit"),x,y), camera_(camera) {
        tooltip_position_ = TooltipPosition::BOTTOM;
    }
    virtual ~SDLQuitButton() {}

    virtual void activate();
private:
    SDLCamera* camera_;
};

/********************************************************************/

class SDLTextButton : public SDLButton {
public:
    SDLTextButton(
        SDLCamera* camera,
        const std::string& text,
        int x, int y,
        const SDL_Color& color = SDLText::black(),
        const SDL_Color& background_color = SDLText::white()
    );
    virtual ~SDLTextButton() {}

private:
    SDLCamera* camera_;
    std::string text_;
};

/********************************************************************/

class SDLToolButton : public SDLButton {
public:
    SDLToolButton(SDLTool* tool, std::string icon_name, int x, int y);
    virtual ~SDLToolButton();

    virtual void activate() override;
    virtual void deactivate() override;
    SDLTool* getTool() const { return tool_; }

private:
    SDLTool* tool_;
};

class SDLItemToolButton : public SDLToolButton {
public:
    SDLItemToolButton(const BasicItem& item, SDLTool* tool, std::string icon_name, int x, int y);
    virtual ~SDLItemToolButton() = default;

    virtual const std::string& text() const override;
    virtual void activate() override;

protected:
    BasicItem item_;
};

class SDLTransformToolButton : public SDLItemToolButton {
public:
    SDLTransformToolButton(const BasicItem& item, SDLTool* tool, std::string icon_name, int x, int y);
    virtual ~SDLTransformToolButton() = default;

    virtual const std::string& text() const override;
};

/********************************************************************/

#endif // sdl_button_h
