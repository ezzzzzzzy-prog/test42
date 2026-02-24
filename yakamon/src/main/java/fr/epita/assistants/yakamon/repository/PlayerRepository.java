package fr.epita.assistants.yakamon.repository;

import fr.epita.assistants.yakamon.data.model.PlayerModel;
import jakarta.enterprise.context.ApplicationScoped;
import jakarta.inject.Inject;
import jakarta.persistence.EntityManager;

@ApplicationScoped
public class PlayerRepository {

    @Inject
    EntityManager em;

    public PlayerModel getPlayer() {
        return em.createQuery("FROM PlayerModel", PlayerModel.class)
                .getResultStream()
                .findFirst()
                .orElse(null);
    }

    public void persist(PlayerModel player) {
        em.persist(player);
    }

    public void deleteAll() {
        em.createQuery("DELETE FROM PlayerModel").executeUpdate();
    }
}

