import random
import glob
import os

usernames = ["alice", "bob", "charlie", "dave",
             "eve", "frank", "grace", "heidi", "ivan", "judy"]
captions = [
    "Enjoying the retro vibes!",
    "Throwback to the 2000s!",
    "Windows XP forever!",
    "Posting from WinClassic!",
    "Just vibing.",
    "Love this aesthetic!",
    "Simple times.",
    "Can’t believe this works!",
    "Testing scrollbar now.",
    "Let’s add more posts!"
]

imgs = []

for file in os.listdir("client/"):
    if file.endswith(".bmp") or file.endswith(".png") or file.endswith(".jpg"):
        imgs.append(file)

with open("client/posts.txt", "w", encoding="utf-8") as f:
    for i in range(30):  # Generate 30 posts
        user = random.choice(usernames)
        caption = random.choice(captions)
        image = random.choice(imgs)
        f.write(f"{user}::{caption}::{image}\n")
