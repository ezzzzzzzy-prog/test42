package fr.epita.assistants.yakamon.domain.service;

import fr.epita.assistants.yakamon.data.model.GameModel;
import fr.epita.assistants.yakamon.data.model.ItemModel;
import fr.epita.assistants.yakamon.data.model.PlayerModel;
import fr.epita.assistants.yakamon.repository.GameRepository;
import fr.epita.assistants.yakamon.repository.ItemRepository;
import fr.epita.assistants.yakamon.repository.PlayerRepository;
import fr.epita.assistants.yakamon.utils.tile.ItemType;
import jakarta.enterprise.context.ApplicationScoped;
import jakarta.inject.Inject;
import jakarta.transaction.Transactional;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.UUID;

@ApplicationScoped
public class GameService {

    @Inject
    PlayerRepository playerRepository;
    @Inject
    ItemRepository itemRepository;
    @Inject
    GameRepository gameRepository;

    @Transactional
    public String startGame(String mapPath, String playerName) {

        // Clear DB
        playerRepository.deleteAll();
        itemRepository.deleteAll();
        gameRepository.deleteAll();

        // Create player
        PlayerModel player = new PlayerModel();
        player.uuid = UUID.randomUUID();
        player.name = playerName;
        player.posX = 0;
        player.posY = 0;
        player.lastMove = null;
        player.lastCollect = null;
        player.lastCatch = null;
        player.lastFeed = null;
        playerRepository.persist(player);

        // Add 5 yakaballs
        ItemModel yakaballs = new ItemModel();
        yakaballs.type = ItemType.YAKABALL;
        yakaballs.quantity = 5;
        itemRepository.persist(yakaballs);

        // Read map file (raw)
        String map;
        try {
            map = Files.readString(Path.of(mapPath));
        } catch (IOException e) {
            throw new IllegalArgumentException("Invalid map path");
        }

        GameModel game = new GameModel();
        game.map = map;
        gameRepository.persist(game);

        return map;
    }

    public boolean isRunning() {
        return gameRepository.exists();
    }
}