package fr.epita.assistants.yakamon.presentation.rest;

import jakarta.ws.rs.*;
import jakarta.ws.rs.core.MediaType;

@Path("/hello")
@Produces(MediaType.TEXT_PLAIN)
@Consumes(MediaType.TEXT_PLAIN)
public class HelloWorldResource {

    @GET
    @Path("/")
    public String helloWorld() {
        return "Hello, world!";
    }

    @GET
    @Path("/{name}")
    public String helloWorld(@PathParam("name") String name) {
        return "Hello " + name + "!";
    }
}
