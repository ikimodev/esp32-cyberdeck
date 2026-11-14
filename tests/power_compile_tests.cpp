// Executed by the compiler as constexpr assertions, not an ESP32 simulator.
#include "../firmware/cyberdeck/cyberdeck_core.h"
#include "../firmware/cyberdeck/oled_renderer.h"
#include "../firmware/cyberdeck/config.h"
using Deck::PowerKey;
using Action=PowerKey::Action;
static_assert(Deck::BOOT_MS==10000 && Deck::POWER_OFF_MS==900,"Preview power deadlines");
static_assert(Deck::OledRenderer::GLITCH_PERIOD_MS==2000 && Deck::OledRenderer::GLITCH_BURST_MS==320,"Preview glitch cadence");
static_assert(sizeof(Deck::Framebuffer)==1024,"One-bit SSD1306 framebuffer");
static_assert(Config::BACK_PIN==27 && Config::POWER_HOLD_MS==2000,"Existing BACK pin, no fifth button");
static_assert([]{PowerKey k;return k.update(true,false,0,true)==Action::None && k.update(true,false,5000,true)==Action::None;}(),"Button held at reset must not toggle power");
static_assert([]{PowerKey k;return k.update(true,true,10,true)==Action::None && k.update(true,false,2009,true)==Action::None && k.update(true,false,2010,true)==Action::Toggle;}(),"Long hold triggers at exactly two seconds");
static_assert([]{PowerKey k;k.update(true,true,0,true);return k.update(true,false,2000,true)==Action::Toggle && k.update(true,false,5000,true)==Action::None && k.update(false,false,5030,true)==Action::None;}(),"Held power emits once and never emits short BACK");
static_assert([]{PowerKey k;return k.update(true,true,0,true)==Action::None && k.update(false,false,500,true)==Action::Back && k.update(false,false,600,true)==Action::None;}(),"Short BACK commits once on release");
static_assert([]{PowerKey k;k.update(true,true,0,false);return k.update(false,false,300,true)==Action::None;}(),"Press started during boot does not become a queued BACK");
static_assert([]{PowerKey k;k.update(true,true,0,false);return k.update(true,false,2000,false)==Action::Toggle && k.update(false,false,2300,false)==Action::None;}(),"Long hold wakes from off and cancels startup");
static_assert([]{PowerKey k;k.update(true,true,0,true);k.update(true,false,2000,true);k.update(false,false,2200,true);k.update(true,true,2500,true);return k.update(false,false,2600,true)==Action::Back;}(),"Release rearms for a new gesture");
static_assert([]{Deck::Button b;b.begin(false,0xfffffff0u);return !b.update(true,0xfffffff5u)&&!b.update(false,0xfffffff8u)&&!b.update(true,0xfffffffau)&&!b.update(true,0x10u)&&b.update(true,0x20u)&&!b.update(true,0x100u)&&b.count==1;}(),"Debounce handles 32-bit millis rollover");
static_assert([]{Deck::Button b;PowerKey k;b.begin(false,0);b.update(true,1);bool edge=b.update(true,31);k.update(b.pressed,edge,31,true);b.update(false,100);b.update(false,130);return k.update(b.pressed,false,130,true)==Action::Back;}(),"Debounced button integrates with the release gesture");
