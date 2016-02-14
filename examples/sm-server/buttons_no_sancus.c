#include "buttons.h"

#include "buttons_process.h"

#include <msp430.h>

#include <stdio.h>

typedef enum
{
    Button1 = 0,
    Button2 = 1
} Button;

enum
{
    Button1Bit = 0x80,
    Button2Bit = 0x20
};

typedef enum
{
    ButtonNotChanged,
    ButtonPressed,
    ButtonReleased
} ButtonEvent;

typedef void (*ButtonCallback)(int);

static int initialized = 0;
static ButtonEvent button1_event, button2_event;
static ButtonCallback callbacks[2];

static ButtonEvent check_button(uint8_t button_bit)
{
    ButtonEvent event = ButtonNotChanged;

    if (!(P1IFG & button_bit))
        return event;

    int down = P1IN & button_bit;

    if (down)
    {
        // IRQ on rising edge.
        P1IES |= button_bit;
        event = ButtonPressed;
    }
    else
    {
        // IRQ on falling edge.
        P1IES &= ~button_bit;
        event = ButtonReleased;
    }

    return event;
}

#undef interrupt
static void __attribute__((interrupt(4))) buttons_isr()
{
    button1_event = check_button(Button1Bit);
    button2_event = check_button(Button2Bit);

    // Clear all PORT1 interrupt flags.
    P1IFG = 0;

    // Notify the event loop if we have events.
    if ((button1_event != ButtonNotChanged) ||
        (button2_event != ButtonNotChanged))
    {
        buttons_process_poll();
    }
}

static void buttons_driver_init()
{
    if (initialized)
        return;

    P1SEL = 0x00;
    P1DIR = 0x00;
    P1IES = 0x00;
    P1IE  = 0xff;

    button1_event = button2_event = ButtonNotChanged;
    initialized = 1;
}

static void do_callback(Button button, ButtonEvent* event)
{
    if (*event == ButtonNotChanged)
        return;

    ButtonCallback cb = callbacks[button];

    // The event will either be handled by the callback or ignored. In either
    // case, we should forget that the event happened now to ensure that we
    // don't try to handle the same event again.
    int pressed = *event == ButtonPressed;
    *event = ButtonNotChanged;

    if (cb == NULL)
        return;

    cb(pressed);
}

static void buttons_driver_handle_events(void)
{
    do_callback(Button1, &button1_event);
    do_callback(Button2, &button2_event);
}

static int buttons_driver_register_callback(Button button, ButtonCallback  cb)
{
    if (button != Button1 && button != Button2)
        return 0;

    callbacks[button] = cb;
    return 1;
}

static void button1_cb(int pressed)
{
    printf("Button1 %s\n", pressed ? "pressed" : "released");
}

void buttons_init()
{
    buttons_driver_init();
    buttons_driver_register_callback(Button1, &button1_cb);
}

void buttons_handle_events()
{
    buttons_driver_handle_events();
}
