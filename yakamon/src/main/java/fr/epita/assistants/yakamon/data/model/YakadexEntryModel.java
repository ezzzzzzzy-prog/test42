package fr.epita.assistants.yakamon.data.model;

import fr.epita.assistants.yakamon.utils.ElementType;
import fr.epita.assistants.yakamon.utils.tile.ItemType;
import jakarta.persistence.*;

@Entity
@Table(name = "yakadex_entry")
public class YakadexEntryModel {

    @Id
    public Integer id;

    public String name;

    @Enumerated(EnumType.STRING)
    public ElementType firstType;

    @Enumerated(EnumType.STRING)
    public ElementType secondType;

    public Integer evolveThreshold;
    public Integer evolutionId;

    public Boolean caught;

    public String description;
}
