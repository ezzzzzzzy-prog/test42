package fr.epita.assistants.yakamon.utils.tile;

import com.fasterxml.jackson.annotation.JsonIgnore;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.annotation.JsonSubTypes;
import com.fasterxml.jackson.annotation.JsonTypeInfo;

@JsonTypeInfo(
        use = JsonTypeInfo.Id.NAME,
        include = JsonTypeInfo.As.PROPERTY,
        property = "type",
        visible = true
)
@JsonSubTypes({
        @JsonSubTypes.Type(value = ItemType.class, name = "ITEM"),
        @JsonSubTypes.Type(value = YakamonType.class, name = "YAKAMON")
})
public interface Collectible {

    @JsonProperty("type")
    CollectibleType getCollectibleType();

    @JsonProperty("value")
    String getValue();

    @JsonIgnore
    CollectibleInfo getCollectibleInfo();
}
