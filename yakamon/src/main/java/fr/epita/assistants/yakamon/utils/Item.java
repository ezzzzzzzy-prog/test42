package fr.epita.assistants.yakamon.utils;

import fr.epita.assistants.yakamon.utils.tile.ItemType;
import lombok.AllArgsConstructor;
import lombok.Data;

@Data
@AllArgsConstructor
public class Item {
    ItemType itemType;
    Integer quantity;
}
