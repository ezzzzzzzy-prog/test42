DELETE FROM yakadex_entry;

INSERT INTO yakadex_entry (id, name, first_type, second_type, description, evolution_id, evolve_threshold, caught)
VALUES
    (3, 'Acumon', 'STELLAR', null, 'Silent and distant, it drifts among the stars. Its light feels warm, yet no one knows what thoughts hide behind its glow.', null, null,false),
    (2, 'Yacumon', 'FIRE', 'DARK', 'Hardened by its falls, it learned to smile even in the dark. Its flames burn quieter, but its will is sharper than ever.', 3, 8, false),
    (1, 'Yakimon', 'FIRE', 'FAIRY', 'It runs anywhere without ever worrying, it may fall but always come backup, nothing seems to alter its joyful spirit.', 2, 2, false),
    (4, 'Louicune', 'WATER', null, 'This mythical creature is the embodiment of the compassion of a pure spring of water, it is said it come on occasion to save desperate people drowning.', null, null, false),
    (5, 'Rayquaïssa', 'DRAGON', 'FLYING', 'He is said to have lived for hundreds of millions of years. Legends remain of how it put to rest the clash between Kyogre and Grouvan.', null, null, false),
    (6, 'Bastiedon', 'ROCK', 'STEEL', 'Steel-faced, unmovable Yakamon that looks defensive but is actually just stubborn, overthinking everything behind an indestructible wall of “I’ve got this.”', null, null, false),
    (8, 'Lucaryon', 'FAIRY','POISON','She has mastered her aura to an even higher level, making her faster, stronger, and deadlier in battle.', null, null, false),
    (7, 'Lucarya', 'FAIRY', null, 'She can sense and control aura, letting her anticipate her opponents’ moves.', 8, 78, false),
    (9, 'Nicotali', 'DARK', null, 'Active only at night, Nicotali silently watches its surroundings.', null, null, false),
    (11, 'Yakoshii', 'GRASS', 'PSYCHIC', 'A silent thread in nature''s loom, it turns the shadows into bloom.', null, null, false),
    (10, 'Koyako', 'GRASS', null, 'Born from the forest''s gentle hum, it dreams beneath the morning sun.', 11, 6, false),
    (13, 'Grouvan', 'GROUND', 'PSYCHIC', 'Quite friendly after all, he is eager to save people from despair.', null, null, false),
    (12, 'Gurvalin', 'GROUND', null, 'Lost in this adventure, he is trying to find his way. He is not afraid of potential enemies.', 13, 15, false);