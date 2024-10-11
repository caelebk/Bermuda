#pragma once
#include "enemy_factories.hpp"

// currently used as "always spawn"
#define TEMP1                                                                  \
  {[this]() {                                                                  \
     createJellyPos(renderer,                                                  \
                    {window_width_px - 550, window_height_px - 250});          \
   },                                                                          \
   [this]() {                                                                  \
     createJellyPos(renderer,                                                  \
                    {window_width_px - 650, window_height_px - 250});          \
   },                                                                          \
   [this]() {                                                                  \
     createJellyPos(renderer,                                                  \
                    {window_width_px - 750, window_height_px - 250});          \
   },                                                                          \
   [this]() {                                                                  \
     createJellyPos(renderer,                                                  \
                    {window_width_px - 850, window_height_px - 250});          \
   },                                                                          \
   [this]() {                                                                  \
     createFishPos(renderer, {window_width_px - 550, window_height_px - 300}); \
   }}

// currently used as "sometimes spawn"
#define TEMP2                                                                  \
  {[this]() {                                                                  \
     createJellyPos(renderer,                                                  \
                    {window_width_px - 650, window_height_px - 350});          \
   },                                                                          \
   [this]() {                                                                  \
     createJellyPos(renderer,                                                  \
                    {window_width_px - 650, window_height_px - 450});          \
   },                                                                          \
   [this]() {                                                                  \
     createJellyPos(renderer,                                                  \
                    {window_width_px - 750, window_height_px - 550});          \
   },                                                                          \
   [this]() {                                                                  \
     createJellyPos(renderer,                                                  \
                    {window_width_px - 850, window_height_px - 650});          \
   },                                                                          \
   [this]() {                                                                  \
     createFishPos(renderer, {window_width_px - 550, window_height_px - 700}); \
   }}

#define TEMP3                                                                  \
  {                                                                            \
      createJellyPos, createJellyPos, createJellyPos,                          \
      createJellyPos, createJellyPos,                                          \
  }
