package fr.epita.assistants.yakamon.converter;

import fr.epita.assistants.yakamon.data.model.ItemModel;
import fr.epita.assistants.yakamon.presentation.api.response.Item;

import jakarta.enterprise.context.ApplicationScoped;

@ApplicationScoped
public class ItemConverter {

    public Item toResponse(ItemModel model) {
        Item item = new Item();
        item.itemType = model.type;
        item.quantity = model.quantity;
        return item;
    }
}

