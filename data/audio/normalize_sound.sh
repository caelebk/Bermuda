#!/bin/bash
mkdir normalized_sounds
for file in ./sound/*.wav; do
    echo "Normalizing $file..."
    filename=$(basename "$file")
    ffmpeg -i "$file" -af "loudnorm=I=-12:TP=-1:LRA=7" "./normalized_sounds/$filename"
done
echo "Normalized Sounds"
