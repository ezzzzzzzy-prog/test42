package fr.epita.assistants.yakamon.utils.tile;

import java.util.HashMap;
import java.util.Map;

public final class CollectibleUtils {
    private CollectibleUtils() {

    }

    private static final Map<Character, Collectible> lookup = new HashMap<>();

    static {
        for (ItemType r : ItemType.values()) {
            lookup.put(r.getCollectibleInfo().getValue(), r);
        }
        for (YakamonType y : YakamonType.values()) {
            lookup.put(y.getCollectibleInfo().getValue(), y);
        }
    }

    public static Collectible getCollectible(Character c) {
        return lookup.get(c);
    }
}