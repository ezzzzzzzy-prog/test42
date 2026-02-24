package fr.epita.assistants.yakamon.presentation.rest;

import fr.epita.assistants.yakamon.converter.YakamonConverter;
import fr.epita.assistants.yakamon.domain.service.TeamService;
import fr.epita.assistants.yakamon.presentation.api.response.YakamonTeamResponse;

import jakarta.inject.Inject;
import jakarta.ws.rs.GET;
import jakarta.ws.rs.Path;
import jakarta.ws.rs.Produces;
import jakarta.ws.rs.core.MediaType;

import java.util.stream.Collectors;

@Path("/team")
@Produces(MediaType.APPLICATION_JSON)
public class TeamResource {

    @Inject
    TeamService teamService;

    @Inject
    YakamonConverter yakamonConverter;

    @GET
    public YakamonTeamResponse getTeam() {

        YakamonTeamResponse response = new YakamonTeamResponse();
        response.yakamons = teamService.getTeam()
                .stream()
                .map(yakamonConverter::toResponse)
                .collect(Collectors.toList());

        return response;
    }
}
