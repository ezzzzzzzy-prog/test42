package fr.epita.assistants.yakamon.utils.tile;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonFormat;
import com.fasterxml.jackson.annotation.JsonIgnore;
import com.fasterxml.jackson.databind.JsonNode;
import lombok.AllArgsConstructor;

@JsonFormat(shape = JsonFormat.Shape.OBJECT)
@AllArgsConstructor
public enum YakamonType implements Collectible {
    GROUVAN(new YakamonInfo('g').withYakadexId(13)),
    GURVALIN(new YakamonInfo('v').withYakadexId(12)),
    YAKOSHII(new YakamonInfo('i').withYakadexId(11)),
    KOYAKO(new YakamonInfo('k').withYakadexId(10)),
    NICOTALI(new YakamonInfo('n').withYakadexId(9)),
    LUCARYON(new YakamonInfo('u').withYakadexId(8)),
    LUCARYA(new YakamonInfo('l').withYakadexId(7)),
    BASTIEDON(new YakamonInfo('b').withYakadexId(6)),
    RAYQUAÏSSA(new YakamonInfo('r').withYakadexId(5)),
    LOUICUNE(new YakamonInfo('o').withYakadexId(4)),
    ACUMON(new YakamonInfo('a').withYakadexId(3)),
    YACUMON(new YakamonInfo('c').withYakadexId(2)),
    YAKIMON(new YakamonInfo('y').withYakadexId(1));

    private final CollectibleInfo collectibleInfo;

    @Override
    public CollectibleType getCollectibleType() {
        return CollectibleType.YAKAMON;
    }

    @Override
    public String getValue() {
        return name();
    }

    @JsonIgnore
    @Override
    public CollectibleInfo getCollectibleInfo() {
        return collectibleInfo;
    }
    @JsonCreator
    public static YakamonType jsonCreator(JsonNode node) {
        String value = node.get("value").asText();
        return YakamonType.valueOf(value);
    }
}
