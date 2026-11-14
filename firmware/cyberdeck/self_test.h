#pragma once
#include "golden_frames.h"
bool puzzleSolvable(Deck::BreachGame game,uint64_t now=0){
 if(game.result=="success")return true;if(!game.result.empty())return false;
 for(int i:game.legal()){auto next=game;next.pick(now,i);if(puzzleSolvable(next,now+1))return true;}return false;
}
bool runSelfTests(){
 int failures=0,frames=0;Deck::Cyberdeck model(true);Deck::OledRenderer screen;
 for(const auto& step:Deck::goldenSteps){
  if(step.reset){model=Deck::Cyberdeck(true);screen=Deck::OledRenderer();}
  if(Deck::Text(step.key)=="power")model.togglePower(step.time);else if(step.key[0])model.input(step.key,step.time);else model.tick(step.time);
  uint32_t actual=screen.render(model).hash();frames++;
  if(actual!=step.hash||model.page!=step.page){failures++;Serial.printf("GOLDEN FAIL frame=%d page=%s expected=%s hash=%08lx expected=%08lx\n",frames,model.page.c_str(),step.page,(unsigned long)actual,(unsigned long)step.hash);if(failures==1){Serial.print("FRAMEHEX ");for(uint8_t b:screen.f.bits)Serial.printf("%02x",b);Serial.println();}}
 }
 for(uint32_t round=0;round<500;round++)if(!puzzleSolvable(Deck::BreachGame(Deck::makePuzzle(2077,round)))){failures++;Serial.printf("PUZZLE FAIL %lu\n",(unsigned long)round);}
 Deck::Button button;button.begin(false,0xfffffff0u);
 if(button.update(true,0xfffffff5u)||button.update(false,0xfffffff8u)||button.update(true,0xfffffffau)||button.update(true,0x10u)||!button.update(true,0x20u)||button.update(true,0x100u)||button.count!=1)failures++;
 if(!Lab::validMessage("HELLO",5)||Lab::validMessage("\xD8\xB9",2))failures++;
 uint8_t bitmap[1024]{};std::string hex(2048,'f');if(!Lab::decodeDrawing(hex.c_str(),hex.size(),bitmap)||bitmap[1023]!=255)failures++;
 Deck::BreachGame g;g.pick(1);size_t used=g.used.size();if(g.pick(2,0)||g.used.size()!=used)failures++;
 Serial.printf("SELFTEST %s: %d golden OLED frames; 500 solvable puzzles; debounce rollover; input validation. failures=%d\n",failures?"FAIL":"PASS",frames,failures);
 return failures==0;
}
