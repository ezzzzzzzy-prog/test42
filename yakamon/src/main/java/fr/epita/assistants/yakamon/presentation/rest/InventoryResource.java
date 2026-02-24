package fr.epita.assistants.yakamon.presentation.rest;
import fr.epita.assistants.yakamon.converter.ItemConverter;
import fr.epita.assistants.yakamon.domain.service.InventoryService;
import fr.epita.assistants.yakamon.presentation.api.response.InventoryResponse;

import jakarta.inject.Inject;
import jakarta.ws.rs.GET;
import jakarta.ws.rs.Path;
import jakarta.ws.rs.Produces;
import jakarta.ws.rs.core.MediaType;

import java.util.stream.Collectors;

@Path("/inventory")
@Produces(MediaType.APPLICATION_JSON)
public class InventoryResource {

    @Inject
    InventoryService inventoryService;

    @Inject
    ItemConverter itemConverter;

    @GET
    public InventoryResponse getInventory() {

        InventoryResponse response = new InventoryResponse();
        response.items = inventoryService.getInventory()
                .stream()
                .map(itemConverter::toResponse)
                .collect(Collectors.toList());

        return response;
    }
}
