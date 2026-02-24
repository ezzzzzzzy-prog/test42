package fr.epita.assistants.yakamon.presentation.rest;

import fr.epita.assistants.yakamon.converter.PlayerConverter;
import fr.epita.assistants.yakamon.data.model.PlayerModel;
import fr.epita.assistants.yakamon.domain.service.PlayerService;
import fr.epita.assistants.yakamon.presentation.api.response.PlayerResponse;
import jakarta.inject.Inject;
import jakarta.ws.rs.GET;
import jakarta.ws.rs.Path;
import jakarta.ws.rs.Produces;
import jakarta.ws.rs.core.MediaType;

@Path("/player")
@Produces(MediaType.APPLICATION_JSON)
public class PlayerResource {

    @Inject
    PlayerService playerService;
    @Inject
    PlayerConverter converter;

    @GET
    public PlayerResponse getPlayer() {
        PlayerModel player = playerService.getPlayer();
        return converter.toResponsep(player);
    }

}
