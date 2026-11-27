# Générateur des sprites pixel art isométriques de Rogue Bike (tuiles 32x16).
# Produit des PNG transparents dans assets/sprites/ : sol (asphalte, pavés), trottoir,
# plaque d'égout (départ), bouche de métro (arrivée), 3 variantes d'immeubles et les personnages
# (motard, voyou en trottinette, voyou en rollers, boss cycliste). Usage : python tools/GenerateSprites.py
# Dépendance : Pillow (pip install pillow). Les PNG générés sont versionnés : ce script
# ne sert qu'à les régénérer ou les modifier.
import random
from pathlib import Path

from PIL import Image, ImageDraw

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


def make_manhole():
    """Plaque d'égout (case de départ) : dalle pavée avec une plaque ronde en fonte quadrillée."""
    rng = random.Random(7)
    image = Image.new("RGBA", (TILE_WIDTH, TILE_HEIGHT), (0, 0, 0, 0))
    pavement = pavement_texture(rng, HALF_H)
    radius_x, radius_y = 8.0, 4.0   # cercle vu en isométrique : ellipse 2:1

    def tile(x, y):
        dx = (x + 0.5 - HALF_W) / radius_x
        dy = (y + 0.5 - HALF_H) / radius_y
        distance = dx * dx + dy * dy
        if distance > 1.0:
            return pavement(x, y)
        if distance > 0.62:
            return (46, 46, 50)            # cerclage
        a, b = iso_coords(x, y, HALF_H)
        if (a * 8) % 1 < 0.3 or (b * 8) % 1 < 0.3:
            return (58, 58, 63)            # rainures du quadrillage
        return (88, 88, 94)                # fonte

    fill_floor(image, HALF_H, tile)
    image.save(OUTPUT_DIR / "manhole.png")


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


# ----- Personnages (vus de profil, tournés vers la droite) -----

# Palette commune des personnages
CHARACTER_COLORS = {
    "k": (18, 18, 22),       # contour
    "f": (226, 180, 140),    # peau
    "e": (40, 30, 30),       # yeux
    "v": (50, 80, 120),      # visière
    "V": (170, 210, 240),    # reflet de visière
    "p": (52, 62, 96),       # jean
    "g": (30, 28, 30),       # gants, chaussures
    "m": (140, 140, 150),    # métal
    "w": (35, 35, 40),       # roues
    "W": (200, 200, 60),     # roues de rollers
}

# Motard à pied : casque intégral rouge, blouson noir
PLAYER = [
    "......kkkkk.....",
    ".....khhhhhk....",
    "....khhhhhhVk...",
    "....khhhhvvvk...",
    "....kHhhhvvvk...",
    "....kHHhhhhHk...",
    ".....kkHHHHk....",
    "....kjjjjjjjk...",
    "...kjjjjjjjjjk..",
    "...kjJjjjjjJjk..",
    "...kjJjjjjjJjk..",
    "...kgJjjjjjJgk..",
    "....kJjjjjjJk...",
    ".....kpppppk....",
    ".....kpppppk....",
    ".....kpp.ppk....",
    ".....kpp.ppk....",
    ".....kpp.ppk....",
    "....kggg.gggk...",
    "....kkkk.kkkk...",
]
PLAYER_COLORS = {"h": (210, 40, 40), "H": (140, 20, 25), "j": (34, 34, 40), "J": (18, 18, 22)}

# Voyou rapide : sweat à capuche vert, trottinette
FAST_ENEMY = [
    ".....kkkkk..........",
    "....kcccccck........",
    "....kccfffck........",
    "....kcffeffk........",
    "....kcfffffk...kkk..",
    ".....kccccck..kmmmk.",
    "....kcccccccckkkmk..",
    "...kcCcccccccffkmk..",
    "...kcCcccccCk.kkmk..",
    "....kCcccccCk...kmk.",
    ".....kpppppk....kmk.",
    ".....kpp.ppk....kmk.",
    ".....kpp.ppk....kmk.",
    ".....kpp.ppk....kmk.",
    "....kggg.gggk...kmk.",
    "..kmmmmmmmmmmmmmmmk.",
    ".kwwk..........kwwk.",
    "..kk............kk..",
]
FAST_COLORS = {"c": (60, 150, 80), "C": (35, 100, 55)}

# Voyou costaud : bonnet, blouson large bordeaux, rollers
HEAVY_ENEMY = [
    "......kkkkkk......",
    ".....kbbbbbbk.....",
    ".....kbBbbBbk.....",
    ".....kffffffk.....",
    ".....kffeffek.....",
    ".....kffffffk.....",
    "....kkjjjjjjkk....",
    "..kkjjjjjjjjjjkk..",
    ".kjjjjjjjjjjjjjjk.",
    ".kjjJjjjjjjjjJjjk.",
    ".kjjJjjjjjjjjJjjk.",
    ".kffJjjjjjjjjJffk.",
    "..kkJjjjjjjjjJkk..",
    "....kppppppppk....",
    "....kppp..pppk....",
    "....kppp..pppk....",
    "....kppp..pppk....",
    "...kgggg..ggggk...",
    "...kmmmm..mmmmk...",
    "...kWkWk..kWkWk...",
]
HEAVY_COLORS = {"b": (40, 40, 48), "B": (200, 60, 60), "j": (120, 30, 40), "J": (80, 18, 26)}

SHADOW_COLOR = (0, 0, 0, 90)


def make_character(name, grid, colors):
    """Personnage dessiné à partir d'une grille de caractères, avec une ombre ovale sous les pieds."""
    width = max(len(row) for row in grid)
    image = Image.new("RGBA", (width, len(grid) + 2), (0, 0, 0, 0))
    ImageDraw.Draw(image).ellipse((1, len(grid) - 2, width - 2, len(grid) + 1), fill=SHADOW_COLOR)
    palette = {**CHARACTER_COLORS, **colors}
    for y, row in enumerate(grid):
        for x, color in enumerate(row):
            if color != ".":
                image.putpixel((x, y), palette[color] + (255,))
    image.save(OUTPUT_DIR / name)


def make_boss():
    """Boss : cycliste en maillot jaune et casque, sur un vélo de course (couvre 2x2 cases)."""
    width, height = 60, 46
    image = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image)
    outline = CHARACTER_COLORS["k"] + (255,)
    frame_color = (30, 110, 200, 255)
    jersey, jersey_dark = (240, 200, 40, 255), (190, 150, 20, 255)
    skin = CHARACTER_COLORS["f"] + (255,)
    rear, front, pedal = (14, 34), (46, 34), (28, 34)
    seat, handle = (23, 20), (41, 18)

    draw.ellipse((4, 40, width - 5, 45), fill=SHADOW_COLOR)
    # Roues : pneu épais, jante, moyeu
    for cx, cy in (rear, front):
        draw.ellipse((cx - 10, cy - 10, cx + 10, cy + 10), outline=outline, width=3)
        draw.ellipse((cx - 7, cy - 7, cx + 7, cy + 7), outline=CHARACTER_COLORS["m"] + (255,), width=1)
        draw.line((cx - 6, cy, cx + 6, cy), fill=CHARACTER_COLORS["m"] + (255,))
        draw.line((cx, cy - 6, cx, cy + 6), fill=CHARACTER_COLORS["m"] + (255,))
        draw.rectangle((cx - 1, cy - 1, cx + 1, cy + 1), fill=outline)
    # Cadre
    for start, end in ((rear, pedal), (pedal, seat), (seat, rear), (pedal, handle), (seat, handle), (handle, front)):
        draw.line((*start, *end), fill=frame_color, width=2)
    draw.line((19, 19, 27, 19), fill=outline, width=2)          # selle
    draw.line((40, 16, 45, 17), fill=outline, width=2)          # guidon
    # Cycliste penché : jambe, buste, bras, tête casquée
    draw.line((24, 20, 31, 28), fill=(30, 30, 36, 255), width=4)   # cuisse (cuissard noir)
    draw.line((31, 28, 28, 35), fill=skin, width=3)                # mollet
    draw.rectangle((26, 35, 31, 36), fill=outline)                 # chaussure
    draw.polygon([(20, 18), (26, 9), (36, 10), (30, 21)], fill=jersey, outline=outline)
    draw.line((27, 15, 33, 12), fill=jersey_dark, width=2)         # bande du maillot
    draw.line((34, 12, 41, 17), fill=skin, width=3)                # bras
    draw.ellipse((33, 2, 43, 11), fill=jersey, outline=outline)    # casque
    draw.line((40, 6, 44, 7), fill=CHARACTER_COLORS["v"] + (255,), width=2)   # lunettes
    image.save(OUTPUT_DIR / "boss.png")


def main():
    """Génère tous les sprites."""
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    make_floor("asphalt.png", asphalt_texture(random.Random(1)))
    make_floor("pavement.png", pavement_texture(random.Random(2), HALF_H))
    make_sidewalk()
    make_manhole()
    make_metro()
    make_character("player.png", PLAYER, PLAYER_COLORS)
    make_character("enemy_fast.png", FAST_ENEMY, FAST_COLORS)
    make_character("enemy_heavy.png", HEAVY_ENEMY, HEAVY_COLORS)
    make_boss()
    make_building("building_brick.png", (150, 72, 56), "brick", 11)
    make_building("building_concrete.png", (142, 142, 138), "concrete", 12)
    make_building("building_stucco.png", (204, 182, 140), "stucco", 13)
    print(f"Sprites générés dans {OUTPUT_DIR}")


if __name__ == "__main__":
    main()
