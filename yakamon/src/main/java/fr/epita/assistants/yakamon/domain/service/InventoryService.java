package fr.epita.assistants.yakamon.domain.service;

import fr.epita.assistants.yakamon.data.model.ItemModel;
import fr.epita.assistants.yakamon.repository.ItemRepository;
import jakarta.enterprise.context.ApplicationScoped;
import jakarta.inject.Inject;

import java.util.List;

@ApplicationScoped
public class InventoryService {

    @Inject
    ItemRepository itemRepository;

    @Inject
    GameService gameService;

    public List<ItemModel> getInventory() {
        if (!gameService.isRunning())
            throw new IllegalStateException("Game not running");

        return itemRepository.findAll();
    }
}
