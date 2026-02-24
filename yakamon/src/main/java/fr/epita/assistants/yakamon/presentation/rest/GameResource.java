package fr.epita.assistants.yakamon.presentation.rest;


import fr.epita.assistants.yakamon.domain.service.GameService;
import fr.epita.assistants.yakamon.presentation.api.response.StartResponse;
import fr.epita.assistants.yakamon.presentation.api.request.StartRequest;

import jakarta.inject.Inject;
import jakarta.ws.rs.*;
import jakarta.ws.rs.core.MediaType;

import java.util.List;

@Path("/start")
@Consumes(MediaType.APPLICATION_JSON)
@Produces(MediaType.APPLICATION_JSON)
public class GameResource {

    @Inject
    GameService gameService;

    @POST
    public StartResponse start(StartRequest request) {
        if (request == null || request.mapPath == null || request.playerName == null)
            throw new WebApplicationException(400);

        gameService.startGame(request.mapPath, request.playerName);

        StartResponse response = new StartResponse();
        response.tiles = List.of(); // beginner placeholder
        return response;
    }
}
