package fr.epita.assistants.yakamon.data.model;
import jakarta.persistence.*;

import java.util.UUID;

@Entity
@Table(name = "yakamon")
public class YakamonModel {

    @Id
    public UUID uuid;

    public String nickname;

    public Integer yakadexId;

    public Integer energyPoints;
}
