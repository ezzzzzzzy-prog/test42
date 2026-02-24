package fr.epita.assistants.yakamon.data.model;

import fr.epita.assistants.yakamon.utils.tile.ItemType;
import jakarta.persistence.*;

@Entity
@Table(name = "item")
public class ItemModel {

    @Id
    @GeneratedValue
    public Integer id;

    @Enumerated(EnumType.STRING)
    public ItemType type;

    public Integer quantity;
}
