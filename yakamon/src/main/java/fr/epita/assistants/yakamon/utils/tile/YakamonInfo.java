package fr.epita.assistants.yakamon.utils.tile;

import lombok.Getter;

@Getter
public class YakamonInfo extends CollectibleInfo {

    public YakamonInfo(Character value) {
        super(value);
    }

    private Integer yakadexId;

    public YakamonInfo withYakadexId(Integer id) {
        this.yakadexId = id;
        return this;
    }
}
