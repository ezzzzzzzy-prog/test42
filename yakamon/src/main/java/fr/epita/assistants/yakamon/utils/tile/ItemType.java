package fr.epita.assistants.yakamon.utils.tile;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonFormat;
import com.fasterxml.jackson.databind.JsonNode;
import lombok.AllArgsConstructor;

@JsonFormat(shape = JsonFormat.Shape.OBJECT)
@AllArgsConstructor
public enum ItemType implements Collectible {
    YAKABALL(new ItemInfo('Y')),
    SCROOGE(new ItemInfo('S')),
    NONE(new ItemInfo('N'));

    private final CollectibleInfo collectibleInfo;

    @Override
    public CollectibleInfo getCollectibleInfo() {
        return collectibleInfo;
    }

    @Override
    public CollectibleType getCollectibleType() {
        return CollectibleType.ITEM;
    }

    public String getValue() {
        return name();
    }

    @JsonCreator
    public static ItemType jsonCreator(JsonNode node) {
        String value = node.get("value").asText();
        return ItemType.valueOf(value);
    }
}

