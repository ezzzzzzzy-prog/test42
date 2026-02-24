package fr.epita.assistants.yakamon.utils.tile;

import fr.epita.assistants.yakamon.utils.ElementType;
import lombok.AllArgsConstructor;
import lombok.Getter;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;

@AllArgsConstructor
@Getter
public enum TerrainType {
    GRASS('G', true, Collections.EMPTY_SET),
    MOUNTAIN('M', false, Set.of(ElementType.ROCK, ElementType.FLYING)),
    ROCK('R', true, Collections.EMPTY_SET),
    SAND('S', true, Collections.EMPTY_SET),
    WATER('W', false, Set.of(ElementType.WATER, ElementType.FLYING)),
    LAVA('L', false, Set.of(ElementType.FIRE, ElementType.FLYING));

    private final Character value;
    private final Boolean walkable;
    private final Set<ElementType> compatibleType;

    private static final Map<Character, TerrainType> lookup;

    static {
        lookup = new HashMap<>();
        for (TerrainType terrainType : TerrainType.values()) {
            lookup.put(terrainType.value, terrainType);
        }
    }

    public static TerrainType getTerrain(Character c) {
        return lookup.get(c);
    }

    public boolean isWalkable() {
        return walkable;
    }

    public boolean isWalkable(ElementType element) {
        return walkable || compatibleType.contains(element);
    }
}
