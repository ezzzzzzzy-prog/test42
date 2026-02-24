package fr.epita.assistants.yakamon.domain.service;

import fr.epita.assistants.yakamon.data.model.YakamonModel;
import fr.epita.assistants.yakamon.repository.YakamonRepository;
import jakarta.enterprise.context.ApplicationScoped;
import jakarta.inject.Inject;

import java.util.List;

@ApplicationScoped
public class TeamService {

    @Inject
    YakamonRepository yakamonRepository;

    @Inject
    GameService gameService;

    public List<YakamonModel> getTeam() {
        if (!gameService.isRunning())
            throw new IllegalStateException("Game not running");

        return yakamonRepository.findAll();
    }
}
