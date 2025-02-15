#include "actrwasm.h"
#include "actrfetch.h"
#include "actrmap.h"
#include "actrcanvas.h"
#include "actrasync.h"
#include "actrjson.h"
#include "actrui.h"
#include "actrlog.h"
#include "actrformat.h"

#define MAP_MAIN 1
#define MAP_PATH "fetch.json.example.clickCount"
// fetch json example

enum MyState
{
    MyStateLoadingMap,
    MyStateMapLoaded
};

struct MyData
{
    int asyncHandle;
    int state;
    struct ActrUIControlButton *button;
    int failCount;
};

struct MyData *data;

void reformatButton()
{
    return;
    actr_free(data->button->label);
    int tapCount = 0;
    if (actr_authenticated())
    {
        tapCount = actr_json_get_int(MAP_MAIN, MAP_PATH);
    }
    struct ActrFormatState *format = actr_format("Click Count: %s");
    actr_format_int(format, tapCount);
    data->button->label = actr_format_close(format);
}
[[clang::export_name("actr_key_down")]]
void actr_key_down(char key)
{
    return;
    actr_log("key down");
    actr_ui_key_down(key);

}
[[clang::export_name("actr_init")]]
void actr_init()
{
    actr_ui_init();
    return;
    data = actr_malloc(sizeof(struct MyData));
    data->state = MyStateLoadingMap;
    data->button = actr_ui_button(20, 20, 200, 50, "loading...");

    if (actr_authenticated())
    {
        actr_log("loading  map");
        data->asyncHandle = actr_json_load(MAP_MAIN);
    }
    else
    {
        reformatButton();
    }
}

[[clang::export_name("actr_pointer_tap")]]
void actr_pointer_tap(int x, int y)
{
    return;
    struct ActrUIControlButton * button = (struct ActrUIControlButton *)actr_ui_tap(x, y);
    if (button == data->button)
    {
        if (actr_authenticated())
        {
            int tapCount = actr_json_get_int(MAP_MAIN, MAP_PATH);
            tapCount++;
            actr_json_set_int(MAP_MAIN, MAP_PATH, tapCount);
            reformatButton();
            actr_json_store(MAP_MAIN);
        }
    }
}

[[clang::export_name("actr_pointer_move")]]
void actr_pointer_move(int x, int y)
{
    return;
    actr_ui_move(x, y);
}

[[clang::export_name("actr_async_result")]]
void actr_async_result(int handle, enum AsyncResult success)
{
    return;
    if (handle == data->asyncHandle)
    {
        actr_log("actr_async_result match handle");
        switch (success)
        {
        case AsyncResultSuccess:
            reformatButton();
            break;
        case AsyncResultFailure:
            // only json_load will fail this way if nothing exists to load
            if (actr_authenticated())
            {
                actr_json_set_int(MAP_MAIN, MAP_PATH, 0);
            }
            reformatButton();
            break;
        case AsyncResultAPIError:
            // json_store/load/delete will fail this way if no user/not logged in or connection failure
            data->failCount++;
            break;
        }
    }
}

[[clang::export_name("actr_step")]]
void actr_step(double delta)
{
    actr_ui_draw(delta);
    return;
    actr_canvas2d_fill_style(255,255,255,100);
    if (actr_authenticated())
    {
        actr_canvas2d_fill_text(20, 90, "User is authenticated. Persistence enabled.");
    }
    else
    {
        actr_canvas2d_fill_text(20, 90, "User not is authenticated. Persistence disabled.");
    }
    struct ActrFormatState *format = actr_format("Load/Store failures: %s");
    actr_format_int(format, data->failCount);
    char * text = actr_format_close(format);
    actr_canvas2d_fill_text(20, 110, text);
    actr_free(text);
}
