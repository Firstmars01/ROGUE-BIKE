# Générateur des sprites pixel art isométriques de Rogue Bike (tuiles 32x16).
# Produit des PNG transparents dans assets/sprites/ : sol (asphalte, pavés), trottoir,
# bouche de métro et 3 variantes d'immeubles. Usage : python tools/GenerateSprites.py
# Dépendance : Pillow (pip install pillow). Les PNG générés sont versionnés : ce script
# ne sert qu'à les régénérer ou les modifier.
import random
from pathlib import Path

from PIL import Image

TILE_WIDTH = 32          # Largeur d'une tuile en pixels
TILE_HEIGHT = 16         # Hauteur d'une tuile en pixels
HALF_W = TILE_WIDTH / 2
HALF_H = TILE_HEIGHT / 2
BUILDING_HEIGHT = 32     # Hauteur des façades d'immeuble
CURB_HEIGHT = 4          # Hauteur de la bordure de trottoir
OUTPUT_DIR = Path(__file__).resolve().parent.parent / "assets" / "sprites"


def diamond_distance(x, y, center_y):
    """Distance normalisée du centre du pixel (x, y) au centre du losange (1 = bord)."""
    return abs(x + 0.5 - HALF_W) / HALF_W + abs(y + 0.5 - center_y) / HALF_H


def iso_coords(x, y, center_y):
    """Coordonnées (a, b) dans le repère de la tuile, chacune dans [0, 1] à l'intérieur du losange."""
    dx = (x + 0.5 - HALF_W) / HALF_W
    dy = (y + 0.5 - center_y) / HALF_H
    return (dx + dy) / 2 + 0.5, (dy - dx) / 2 + 0.5


def shade(color, amount):
    """Éclaircit (amount > 0) ou assombrit (amount < 0) une couleur RGB, bornée à [0, 255]."""
    return tuple(max(0, min(255, c + amount)) for c in color)


def face_at(x, y, top_center_y, height):
    """Renvoie ('left'|'right', u, v) si le pixel est sur une face verticale, sinon None.
    u : position horizontale sur la face (0..16) ; v : hauteur sous l'arête du dessus (0..height)."""
    cx, cy = x + 0.5, y + 0.5
    if cx < HALF_W:
        edge = top_center_y + cx / 2
        side, u = "left", cx
    else:
        edge = top_center_y + HALF_H - (cx - HALF_W) / 2
        side, u = "right", cx - HALF_W
    v = cy - edge
    return (side, u, v) if 0 <= v <= height else None


def fill_floor(image, center_y, texture):
    """Remplit le losange de centre vertical center_y avec texture(x, y) -> couleur RGB."""
    for y in range(image.height):
        for x in range(image.width):
            if diamond_distance(x, y, center_y) <= 1.0:
                image.putpixel((x, y), texture(x, y) + (255,))


def asphalt_texture(rng):
    """Asphalte gris foncé grainé."""
    base = (58, 58, 63)
    return lambda x, y: shade(base, rng.choice((-8, -4, 0, 0, 0, 4, 9)))


def pavement_texture(rng, center_y):
    """Pavés beiges 4x4 par tuile, joints sombres, légère variation par pavé."""
    stone_shades = {}

    def texture(x, y):
        a, b = iso_coords(x, y, center_y)
        if (a * 4) % 1 < 0.14 or (b * 4) % 1 < 0.14:
            return (92, 84, 76)
        stone = (int(a * 4), int(b * 4))
        stone_shades.setdefault(stone, rng.choice((-10, -5, 0, 5, 8)))
        return shade((132, 121, 106), stone_shades[stone] + rng.choice((-3, 0, 0, 3)))

    return texture


def make_floor(name, texture_factory):
    """Crée un sprite de sol 32x16."""
    image = Image.new("RGBA", (TILE_WIDTH, TILE_HEIGHT), (0, 0, 0, 0))
    fill_floor(image, HALF_H, texture_factory)
    image.save(OUTPUT_DIR / name)


def make_sidewalk():
    """Trottoir gris : dalle claire avec joints, bordure de CURB_HEIGHT pixels."""
    rng = random.Random(3)
    image = Image.new("RGBA", (TILE_WIDTH, TILE_HEIGHT + CURB_HEIGHT), (0, 0, 0, 0))
    for y in range(image.height):
        for x in range(image.width):
            face = face_at(x, y, HALF_H, CURB_HEIGHT)
            if face:
                color = (126, 126, 122) if face[0] == "left" else (98, 98, 95)
                image.putpixel((x, y), color + (255,))

    def slab(x, y):
        a, b = iso_coords(x, y, HALF_H)
        if (a * 2) % 1 < 0.08 or (b * 2) % 1 < 0.08:
            return (128, 128, 124)
        return shade((168, 168, 163), rng.choice((-4, 0, 0, 3)))

    fill_floor(image, HALF_H, slab)
    image.save(OUTPUT_DIR / "sidewalk.png")


def make_building(name, wall_color, pattern, seed):
    """Immeuble : toit plat avec rebord, deux façades à fenêtres (gauche éclairée, droite à l'ombre)."""
    rng = random.Random(seed)
    image = Image.new("RGBA", (TILE_WIDTH, TILE_HEIGHT + BUILDING_HEIGHT), (0, 0, 0, 0))
    lit_windows = {}
    for y in range(image.height):
        for x in range(image.width):
            face = face_at(x, y, HALF_H, BUILDING_HEIGHT)
            if not face:
                continue
            side, u, v = face
            color = wall_color if side == "left" else shade(wall_color, -28)
            # Motif du matériau (briques, panneaux de béton, crépi)
            if pattern == "brick" and int(v) % 3 == 0:
                color = shade(color, -18)
            elif pattern == "concrete" and int(v) % 10 == 0:
                color = shade(color, -14)
            else:
                color = shade(color, rng.choice((-3, 0, 0, 3)))
            # Fenêtres : 3 colonnes par façade, une rangée par étage
            column, row = int(u // 5.4), int(v // 10)
            if 1.2 <= u % 5.4 <= 3.6 and 3 <= v % 10 <= 7 and v < BUILDING_HEIGHT - 2:
                lit = lit_windows.setdefault((side, column, row), rng.random() < 0.35)
                color = (236, 204, 110) if lit else (44, 58, 84)
                if side == "right":
                    color = shade(color, -25)
            # Arête verticale avant et bas de façade
            if abs(x + 0.5 - HALF_W) < 1.0 or v > BUILDING_HEIGHT - 1.5:
                color = shade(color, -35)
            image.putpixel((x, y), color + (255,))

    def roof(x, y):
        distance = diamond_distance(x, y, HALF_H)
        return (150, 146, 140) if distance > 0.78 else shade((84, 82, 80), rng.choice((-3, 0, 3)))

    fill_floor(image, HALF_H, roof)
    image.save(OUTPUT_DIR / name)


M_LETTER = ["1...1", "11.11", "1.1.1", "1...1", "1...1"]


def make_metro():
    """Bouche de métro : dalle pavée, trémie avec marches, rambarde et panneau « M »."""
    rng = random.Random(5)
    image = Image.new("RGBA", (TILE_WIDTH, TILE_WIDTH), (0, 0, 0, 0))
    base_center_y = TILE_WIDTH - HALF_H
    pavement = pavement_texture(rng, base_center_y)

    def tile(x, y):
        a, b = iso_coords(x, y, base_center_y)
        if 0.22 <= a <= 0.78 and 0.22 <= b <= 0.78:
            if a < 0.27 or b < 0.27:
                return (190, 190, 186)          # rambarde
            step = int((a - 0.22) / 0.56 * 6)
            return (36, 36, 42) if step % 2 else (68, 68, 76)  # marches qui descendent
        return pavement(x, y)

    fill_floor(image, base_center_y, tile)
    # Poteau et panneau
    for y in range(7, 21):
        for x in (6, 7):
            image.putpixel((x, y), (60, 60, 64, 255))
    for y in range(1, 9):
        for x in range(2, 12):
            image.putpixel((x, y), (32, 120, 72, 255))
    for row, line in enumerate(M_LETTER):
        for column, pixel in enumerate(line):
            if pixel == "1":
                image.putpixel((4 + column, 2 + row), (250, 250, 245, 255))
    image.save(OUTPUT_DIR / "metro.png")


def main():
    """Génère tous les sprites."""
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    make_floor("asphalt.png", asphalt_texture(random.Random(1)))
    make_floor("pavement.png", pavement_texture(random.Random(2), HALF_H))
    make_sidewalk()
    make_metro()
    make_building("building_brick.png", (150, 72, 56), "brick", 11)
    make_building("building_concrete.png", (142, 142, 138), "concrete", 12)
    make_building("building_stucco.png", (204, 182, 140), "stucco", 13)
    print(f"Sprites générés dans {OUTPUT_DIR}")


if __name__ == "__main__":
    main()
