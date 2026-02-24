package fr.epita.assistants.yakamon.data.model;
import jakarta.persistence.*;

@Entity
@Table(name = "game")
public class GameModel {

    @Id
    @GeneratedValue
    public Integer id;

    @Column(columnDefinition = "TEXT")
    public String map;
}
