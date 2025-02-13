#ifndef ACTRUI_H
#define ACTRUI_H
#include "actrwasm.h"
#include "actrquadtree.h"
#include "actrcanvas.h"
#include "actrhashtable.h"

struct ActrUIState
{
    struct ActrHashTable *controls;
    struct ActrVector *results;
    struct ActrQuadTree *tree;
    int sequence;
    int hover;
    int focus;
};

enum ActrUIType
{
    ActrUITypeButton,
    ActrUITypeText,
    ActrUITypeContainer
};

struct ActrUIControl
{
    enum ActrUIType type;
    int identity;
    struct ActrQuadTreeLeaf *leaf;
    int hidden;
    struct ActrUIControlContainer *container;
};
struct ActrUIControlButton
{
    // shared with ActrUIControl
    enum ActrUIType type;
    int identity;
    struct ActrQuadTreeLeaf *leaf;
    int hidden;
    struct ActrUIControlContainer *container;
    // unique
    char *label;
};
struct ActrUIControlText
{
    // shared with ActrUIControl
    enum ActrUIType type;
    int identity;
    struct ActrQuadTreeLeaf *leaf;
    int hidden;
    struct ActrUIControlContainer *container;
    // unique
    char *value;
    int cursor;
};
struct ActrUIControlContainer
{
    // shared with ActrUIControl
    enum ActrUIType type;
    int identity;
    struct ActrQuadTreeLeaf *leaf;
    int hidden;
    struct ActrUIControlContainer *container;
};

void _actr_ui_button_dispose(struct ActrUIControlButton *button);
void _actr_ui_text_dispose(struct ActrUIControlText *text);

struct ActrUIState *_actr_ui_state;

void actr_ui_init()
{
    _actr_ui_state = actr_malloc(sizeof(struct ActrUIState));
    _actr_ui_state->controls = actr_hash_table_init();
    _actr_ui_state->results = actr_vector_init(8, 8);
    _actr_ui_state->tree = actr_quad_tree_init(1, 256, 256, 64, 0);
    _actr_ui_state->sequence = 1;
    _actr_ui_state->focus = 0;
    _actr_ui_state->hover = 0;
}
extern void uibutton(int key);

struct ActrUIControl *actr_ui_get_control(int identity)
{
    return actr_hash_table_find(_actr_ui_state->controls, identity);
}

void actr_ui_remove_control(int identity)
{
    struct ActrUIControl *control = actr_ui_get_control(identity);
    actr_quad_tree_remove(control->leaf);
    switch (control->type)
    {
    case ActrUITypeButton:
        _actr_ui_button_dispose(control);
        break;
    case ActrUITypeText:
        _actr_ui_text_dispose(control);
        break;
    }
    actr_hash_table_delete(_actr_ui_state->controls, identity);
}
void actr_ui_key_down_text(struct ActrUIControlText *text, int key)
{
    uibutton(key);
    int newLength;
    char *newValue;
    int currentLength = strlen(text->value);
    if (key == 1)
    {
        // left arrow
        text->cursor--;
        if (text->cursor < 0)
        {
            text->cursor = 0;
        }
    }
    else if (key == 2)
    {
        // right arrow
        text->cursor++;
        if (text->cursor > currentLength)
        {
            text->cursor = currentLength;
        }
    }
    else if (key == 3)
    {
        // backspace
        if (text->cursor == 0)
        {
            return;
        }
        newLength = currentLength - 1;
        newValue = actr_malloc(newLength + 1);
        for (int i = 0; i < newLength; i++)
        {
            if (i < text->cursor - 1)
            {
                newValue[i] = text->value[i];
            }
            else
            {
                newValue[i] = text->value[i + 1];
            }
        }
        actr_free(text->value);
        text->value = newValue;
        text->cursor--;
    }
    else if (key == 4)
    {
        // delete
        if (text->cursor == currentLength)
        {
            return;
        }
        newLength = currentLength - 1;
        newValue = actr_malloc(newLength + 1);
        for (int i = 0; i < newLength; i++)
        {
            if (i <= text->cursor - 1)
            {
                newValue[i] = text->value[i];
            }
            else
            {
                newValue[i] = text->value[i + 1];
            }
        }
        actr_free(text->value);
        text->value = newValue;
    }
    else if (key >= 32 && key <= 126)
    {
        newLength = currentLength + 1;
        newValue = actr_malloc(newLength + 1);

        for (int i = 0; i < newLength; i++)
        {
            if (i < text->cursor)
            {
                newValue[i] = text->value[i];
            }
            else if (i == text->cursor)
            {
                newValue[i] = key;
            }
            else
            {
                newValue[i] = text->value[i - 1];
            }
        }
        actr_free(text->value);
        text->value = newValue;
        text->cursor++;
    }
}
int actr_ui_key_down(int key)
{

    // key
    // 1 left arrow
    // 2 right arrow
    // 3 backspace
    //  - 31 are available
    // when key >= 32 and key <= 126 normal ascii codes
    struct ActrUIControl *control;

    int focus = _actr_ui_state->focus;
    if (focus < 1)
    {
        return focus;
    }
    control = (struct ActrUIControl *)actr_hash_table_find(_actr_ui_state->controls, focus);
    switch (control->type)
    {
    case ActrUITypeButton:
        break;
    case ActrUITypeText:
        actr_ui_key_down_text((struct ActrUIControlText *)control, key);
        break;
    }
    return focus;
}

void _actr_ui_query(int x, int y, int w, int h)
{
    struct ActrQuadTreeBounds area;
    area.point.x = x;
    area.point.y = y;
    area.size.w = w;
    area.size.h = h;
    actr_quad_tree_query(_actr_ui_state->tree, &area, _actr_ui_state->results);
}
void actr_ui_move(int x, int y)
{
    _actr_ui_query(x, y, 1, 1);

    int identity = -1;
    for (int i = 0; i < _actr_ui_state->results->count; i++)
    {
        struct ActrUIControl *type = (struct ActrUIControl *)_actr_ui_state->results->head[i];
        if (type->identity > identity)
        {
            identity = type->identity;
        }
    }
    _actr_ui_state->hover = identity;
    _actr_ui_state->results->count = 0;
}
void _actr_ui_set_focus(int identity)
{
    if (identity < 1)
    {
        _actr_ui_state->focus = 0;
        return;
    }
    if (_actr_ui_state->focus == identity)
    {
        return;
    }
    _actr_ui_state->focus = identity;
    struct ActrUIControl *control = (struct ActrUIControl *)actr_hash_table_find(_actr_ui_state->controls, identity);
    if (control->type == ActrUITypeText)
    {
        struct ActrUIControlText *text = (struct ActrUIControlText *)control;
        text->cursor = strlen(text->value);
    }
}
int actr_ui_tap(int x, int y)
{
    _actr_ui_query(x, y, 1, 1);

    int identity = 0;
    for (int i = 0; i < _actr_ui_state->results->count; i++)
    {
        struct ActrUIControl *type = (struct ActrUIControl *)_actr_ui_state->results->head[i];
        if (type->identity > identity)
        {
            identity = type->identity;
        }
    }

    _actr_ui_state->results->count = 0;
    _actr_ui_set_focus(identity);
    return identity;
}
void free_zero_leaf();
void free_zero_label();
void free_zero_button();
void _actr_ui_button_dispose(struct ActrUIControlButton *button)
{
    if (!button->leaf)
    {
        free_zero_leaf();
    }
    actr_free(button->leaf);
    
    if (!button->label)
    {
        free_zero_label();
    }
    actr_free(button->label);
        
    if (!button)
    {
        free_zero_button();
    }
    actr_free(button);
}
struct ActrUIControlButton *actr_ui_button(int x, int y, int w, int h, char *label)
{
    struct ActrUIControlButton *button = actr_malloc(sizeof(struct ActrUIControlButton));

    button->type = ActrUITypeButton;
    button->identity = _actr_ui_state->sequence++;
    button->leaf = actr_quad_tree_leaf(x, y, w, h, button);
    button->label = actr_heap_string(label);

    actr_quad_tree_insert(_actr_ui_state->tree, button->leaf);
    actr_hash_table_insert(_actr_ui_state->controls, button->identity, button);
    return button;
}

void _actr_ui_text_dispose(struct ActrUIControlText *text)
{
    actr_free(text->leaf);
    actr_free(text->value);
    actr_free(text);
}
struct ActrUIControlText *actr_ui_text(int x, int y, int w, int h, char *value)
{
    struct ActrUIControlText *text = actr_malloc(sizeof(struct ActrUIControlText));

    text->type = ActrUITypeText;
    text->identity = _actr_ui_state->sequence++;
    text->leaf = actr_quad_tree_leaf(x, y, w, h, text);
    text->value = actr_heap_string(value);

    actr_quad_tree_insert(_actr_ui_state->tree, text->leaf);
    actr_hash_table_insert(_actr_ui_state->controls, text->identity ,text);
    return text;
}

void _actr_ui_set_focus_style(int focused)
{
    if (focused)
    {
        actr_canvas2d_stroke_style(150, 250, 150, 100);
    }
    else
    {
        actr_canvas2d_stroke_style(200, 200, 200, 100);
    }
}
void _actr_ui_set_hover_style(int hovered)
{
    if (hovered)
    {
        actr_canvas2d_fill_style(150, 200, 150, 100);
    }
    else
    {
        actr_canvas2d_fill_style(150, 150, 150, 100);
    }
}
struct ActrPoint *_actr_ui_get_control_position(struct ActrUIControl *control)
{
    struct ActrPoint *result = actr_malloc(sizeof(struct ActrPoint));

    result->x = control->leaf->bounds.point.x;
    result->y = control->leaf->bounds.point.y;

    control = control->container;

    while (control)
    {
        result->x += control->leaf->bounds.point.x;
        result->y += control->leaf->bounds.point.y;
        control = control->container;
    }
    return result;
}
void _actr_ui_draw_text(struct ActrUIControlText *text)
{
    struct ActrPoint *position;
    struct ActrSize *size = &text->leaf->bounds.size;
    if (text->container)
    {
        position = _actr_ui_get_control_position(text);
    }
    else
    {
        position = &text->leaf->bounds.point;
    }
    int hovered = _actr_ui_state->hover == text->identity;
    int focused = _actr_ui_state->focus == text->identity;

    _actr_ui_set_hover_style(hovered && !focused);

    actr_canvas2d_fill_rect(position->x, position->y, size->w, size->h);

    actr_canvas2d_fill_style(0, 0, 255, 100);
    int charWidth = 9;
    int padSide = 5;
    int maxChars = (size->w - padSide * 2) / charWidth;
    int halfChars = maxChars / 2;
    int charCount = strlen(text->value);
    int substart = text->cursor - halfChars;
    if (charCount > maxChars)
    {
        if (substart < 0)
        {
            substart = 0;
        }
        if (substart + maxChars > charCount)
        {
            substart -= substart + maxChars - charCount;
        }
        char *display = substr(text->value, substart, maxChars);
        actr_canvas2d_fill_text(position->x + padSide, position->y + size->h - 5, display);
        actr_free(display);
    }
    else
    {
        substart = 0;
        actr_canvas2d_fill_text(position->x + padSide, position->y + size->h - 5, text->value);
    }

    _actr_ui_set_focus_style(focused);
    actr_canvas2d_stroke_rect(position->x, position->y, size->w, size->h);

    if (focused)
    {
        actr_canvas2d_stroke_style(0, 0, 255, 100);
        actr_canvas2d_begin_path();

        int cursorStart = position->x + 5;
        actr_canvas_moveto(cursorStart + (text->cursor - substart) * charWidth, position->y + size->h - 3);
        actr_canvas_lineto(cursorStart + (text->cursor + 1 - substart) * charWidth, position->y + size->h - 3);
        actr_canvas2d_stroke();
    }

    if (text->container)
    {
        actr_free(position);
    }
}

void _actr_ui_draw_button(struct ActrUIControlButton *button)
{
    struct ActrQuadTreeBounds *bounds = &button->leaf->bounds;

    int focused = _actr_ui_state->focus == button->identity;
    int hovered = _actr_ui_state->hover == button->identity;
    _actr_ui_set_hover_style(hovered && !focused);

    actr_canvas2d_fill_rect(bounds->point.x, bounds->point.y, bounds->size.w, bounds->size.h);

    actr_canvas2d_fill_style(0, 0, 0, 100);
    int charWidth = 9;
    int padSide = 5;
    int maxChars = (bounds->size.w - padSide * 2) / charWidth;
    int charCount = strlen(button->label);

    if (charCount > maxChars)
    {
        char *label = substr(button->label, 0, maxChars);
        actr_canvas2d_fill_text(bounds->point.x + padSide, bounds->point.y + bounds->size.h - 5, label);
        actr_free(label);
    }
    else
    {
        actr_canvas2d_fill_text(bounds->point.x + padSide, bounds->point.y + bounds->size.h - 5, button->label);
    }

    if (focused)
    {
        actr_canvas2d_stroke_style(200, 250, 200, 100);
    }
    else
    {
        actr_canvas2d_stroke_style(200, 200, 200, 100);
    }
    actr_canvas2d_stroke_rect(bounds->point.x, bounds->point.y, bounds->size.w, bounds->size.h);
}
void actr_ui_draw(double delta)
{
    // clear canvas
    actr_canvas2d_fill_style(0, 0, 0, 100);
    actr_canvas2d_fill_rect(-10, -10, actrState->canvasSize.w + 20, actrState->canvasSize.h + 20);

    _actr_ui_query(0, 0, actrState->canvasSize.w, actrState->canvasSize.h);

    for (int i = 0; i < _actr_ui_state->results->count; i++)
    {
        struct ActrUIControl *control = (struct ActrUIControl *)_actr_ui_state->results->head[i];
        if (control->hidden)
        {
            continue;
        }
        int hidden = 0;
        struct ActrUIControlContainer *container = control->container;
        while (container)
        {
            if (container->hidden)
            {
                hidden = 1;
                break;
            }
            container = container->container;
        }
        if (hidden)
        {
            continue;
            ;
        }
        switch (control->type)
        {
        case ActrUITypeButton:
            _actr_ui_draw_button(_actr_ui_state->results->head[i]);
            break;
        case ActrUITypeText:
            _actr_ui_draw_text(_actr_ui_state->results->head[i]);
            break;
        }
    }

    _actr_ui_state->results->count = 0;

    // draw memory report
    char *mem = actr_memory_report();
    actr_canvas2d_measure_text(mem);
    actr_canvas2d_fill_style(0, 0, 0, 100);

    actr_canvas2d_fill_rect(0, 0, actrState->textSize.x + 2, actrState->textSize.y + 3);
    actr_canvas2d_fill_style(255, 255, 255, 100);
    actr_canvas2d_fill_text(1, actrState->textSize.y, mem);
    actr_free(mem);

    actr_quad_tree_draw(_actr_ui_state->tree);
}

#endif
