package fr.epita.assistants.yakamon.repository;
import fr.epita.assistants.yakamon.data.model.YakamonModel;
import jakarta.enterprise.context.ApplicationScoped;
import jakarta.inject.Inject;
import jakarta.persistence.EntityManager;

import java.util.List;

@ApplicationScoped
public class YakamonRepository {

    @Inject
    EntityManager em;

    public List<YakamonModel> findAll() {
        return em.createQuery("FROM YakamonModel", YakamonModel.class)
                .getResultList();
    }
}
