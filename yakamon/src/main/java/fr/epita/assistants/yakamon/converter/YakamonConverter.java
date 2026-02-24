package fr.epita.assistants.yakamon.converter;
import fr.epita.assistants.yakamon.data.model.YakamonModel;
import fr.epita.assistants.yakamon.presentation.api.response.YakamonResponse;
import jakarta.enterprise.context.ApplicationScoped;


@ApplicationScoped
public class YakamonConverter {

    public YakamonResponse toResponse(YakamonModel model) {
        YakamonResponse r = new YakamonResponse();
        r.uuid = model.uuid;
        r.nickname = model.nickname;
        r.yakadexId = model.yakadexId;
        r.energyPoints = model.energyPoints;
        return r;
    }
}
