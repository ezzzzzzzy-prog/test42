package fr.epita.assistants.yakamon.domain.service;

import fr.epita.assistants.yakamon.data.model.PlayerModel;
import fr.epita.assistants.yakamon.repository.PlayerRepository;
import jakarta.enterprise.context.ApplicationScoped;
import jakarta.inject.Inject;

@ApplicationScoped
public class PlayerService {

    @Inject
    PlayerRepository playerRepository;
    @Inject
    GameService gameService;

    public PlayerModel getPlayer() {
        if (!gameService.isRunning())
            throw new IllegalStateException("Game not running");

        return playerRepository.getPlayer();
    }
}
