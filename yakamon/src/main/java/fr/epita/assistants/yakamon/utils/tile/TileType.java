package fr.epita.assistants.yakamon.utils.tile;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;
import lombok.AllArgsConstructor;
import lombok.Data;

@Data
@AllArgsConstructor
public class TileType {

    private TerrainType terrainType;

    private Collectible collectible;

    @JsonCreator
    public TileType(
            @JsonProperty("collectible") Collectible type
    ) {
        this.collectible = type;
    }
}
