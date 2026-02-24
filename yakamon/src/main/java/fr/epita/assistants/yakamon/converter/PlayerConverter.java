package fr.epita.assistants.yakamon.converter;

import fr.epita.assistants.yakamon.data.model.PlayerModel;
import fr.epita.assistants.yakamon.presentation.api.response.PlayerResponse;
import jakarta.enterprise.context.ApplicationScoped;

@ApplicationScoped
public class PlayerConverter {

    public PlayerResponse toResponsep(PlayerModel model) {
        PlayerResponse r = new PlayerResponse();
        r.uuid = model.uuid;
        r.name = model.name;
        r.posX = model.posX;
        r.posY = model.posY;
        r.lastMove = model.lastMove;
        r.lastCollect = model.lastCollect;
        r.lastCatch = model.lastCatch;
        r.lastFeed = model.lastFeed;
        return r;
    }
}
