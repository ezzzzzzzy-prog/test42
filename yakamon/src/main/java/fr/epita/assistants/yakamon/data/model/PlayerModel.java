package fr.epita.assistants.yakamon.data.model;

import jakarta.persistence.Entity;
import jakarta.persistence.Id;
import jakarta.persistence.Table;

import java.time.LocalDateTime;
import java.util.UUID;

@Entity
@Table(name = "player")
public class PlayerModel {

    @Id
    public UUID uuid;

    public String name;

    public Integer posX;
    public Integer posY;

    public LocalDateTime lastMove;
    public LocalDateTime lastCatch;
    public LocalDateTime lastCollect;
    public LocalDateTime lastFeed;
}
