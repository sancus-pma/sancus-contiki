#include "buttons_driver.h"

#include "port1_mmio.h"

#include <stdio.h>

// This is declared in buttons_process.h but we don't want to include that
// header since it generates lot's of warnings by including Contiki headers.
// That's also the reason this function exists in the first place instead of
// just calling process_poll(...).
void buttons_process_poll(void);

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

SM_DATA(buttons_driver) int initialized = 0;
SM_DATA(buttons_driver) volatile ButtonEvent button1_event, button2_event;

SM_FUNC(buttons_driver) ButtonEvent check_button(port1_data_t* data,
                                                 uint8_t button_bit)
{
    ButtonEvent event = ButtonNotChanged;

    if (!(R_P1IFG(*data) & button_bit))
        return event;

    int down = R_P1IN(*data) & button_bit;

    if (down)
    {
        // IRQ on rising edge.
        *data |= W_P1IES(button_bit);
        event = ButtonPressed;
    }
    else
    {
        // IRQ on falling edge.
        *data &= ~W_P1IES(button_bit);
        event = ButtonReleased;
    }

    return event;
}

SM_ISR(buttons_driver, num)
{
    if (num != 2)
    {
        puts("Wrong IRQ");
        return;
    }

    port1_data_t data = port1_mmio_read();
    button1_event = check_button(&data, Button1Bit);
    button2_event = check_button(&data, Button2Bit);

    // Clear all PORT1 interrupt flags.
    data &= ~W_P1IFG(0xff);
    port1_mmio_write(data);

    // Notify the event loop if we have events.
    if ((button1_event != ButtonNotChanged) ||
        (button2_event != ButtonNotChanged))
    {
        buttons_process_poll();
    }
}

SM_HANDLE_IRQ(buttons_driver, 2);

SM_ENTRY(buttons_driver) void buttons_driver_init()
{
    if (initialized)
        return;

    port1_data_t config = port1_mmio_read();
    config &= ~W_P1SEL(0xff);
    config &= ~W_P1DIR(0xff);
    config &= ~W_P1IES(0xff);
    config |=  W_P1IE(0xff);
    port1_mmio_write(config);

    button1_event = button2_event = ButtonNotChanged;
    initialized = 1;
}

SM_ENTRY(buttons_driver) void buttons_driver_handle_events(void)
{
    puts("Handling events");
}

DECLARE_SM(buttons_driver, 0x1234);
