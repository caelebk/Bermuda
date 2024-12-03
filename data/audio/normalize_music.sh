#!/bin/bash
mkdir normalized_music
for file in ./music/*.wav; do
    echo "Normalizing $file..."
    filename=$(basename "$file")
    ffmpeg -i "$file" -af "loudnorm=I=-18:TP=-1:LRA=7" -ar 22050 "./normalized_music/$filename"
done
echo "Normalized Music"
