#pragma once
#include "enemy_factories.hpp"

#define LEVEL_1_ENEMIES_FIXED                                                  \
  {[this]() { createFishRoom(renderer, room_random); },                        \
   [this]() { createFishRoom(renderer, room_random); },                        \
   [this]() { createFishRoom(renderer, room_random); },                        \
   [this]() { createJellyRoom(renderer, room_random); },                       \
   [this]() { createJellyRoom(renderer, room_random); },                       \
   [this]() {                                                                  \
     createJellyPos(renderer,                                                  \
                    {window_width_px - 450, window_height_px - 250});          \
   },                                                                          \
   [this]() {                                                                  \
     createFishPos(renderer, {window_width_px - 450, window_height_px - 300}); \
   }}
#define LEVEL_1_ENEMIES_RAND                                                   \
  {                                                                            \
      [this]() { createFishRoom(renderer, room_random); },                     \
      [this]() { createFishRoom(renderer, room_random); },                     \
      [this]() { createFishRoom(renderer, room_random); },                     \
      [this]() { createJellyRoom(renderer, room_random); },                    \
      [this]() { createJellyRoom(renderer, room_random); },                    \
  }
#define TEMP1                                                                  \
  {[this]() {                                                                  \
     createJellyPos(renderer,                                                  \
                    {window_width_px - 150, window_height_px - 250});          \
   },                                                                          \
   [this]() {                                                                  \
     createJellyPos(renderer,                                                  \
                    {window_width_px - 250, window_height_px - 250});          \
   },                                                                          \
   [this]() {                                                                  \
     createJellyPos(renderer,                                                  \
                    {window_width_px - 350, window_height_px - 250});          \
   },                                                                          \
   [this]() {                                                                  \
     createJellyPos(renderer,                                                  \
                    {window_width_px - 450, window_height_px - 250});          \
   },                                                                          \
   [this]() {                                                                  \
     createFishPos(renderer, {window_width_px - 550, window_height_px - 300}); \
   }}

#define TEMP2                                                                  \
  {[this]() {                                                                  \
     createJellyPos(renderer,                                                  \
                    {window_width_px - 150, window_height_px - 350});          \
   },                                                                          \
   [this]() {                                                                  \
     createJellyPos(renderer,                                                  \
                    {window_width_px - 250, window_height_px - 450});          \
   },                                                                          \
   [this]() {                                                                  \
     createJellyPos(renderer,                                                  \
                    {window_width_px - 350, window_height_px - 550});          \
   },                                                                          \
   [this]() {                                                                  \
     createJellyPos(renderer,                                                  \
                    {window_width_px - 450, window_height_px - 650});          \
   },                                                                          \
   [this]() {                                                                  \
     createFishPos(renderer, {window_width_px - 550, window_height_px - 700}); \
   }}
