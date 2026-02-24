package fr.epita.assistants.yakamon.repository;
import fr.epita.assistants.yakamon.data.model.GameModel;
import jakarta.enterprise.context.ApplicationScoped;
import jakarta.inject.Inject;
import jakarta.persistence.EntityManager;

@ApplicationScoped
public class GameRepository {

    @Inject
    EntityManager em;

    public void persist(GameModel game) {
        em.persist(game);
    }

    public void deleteAll() {
        em.createQuery("DELETE FROM GameModel").executeUpdate();
    }

    public boolean exists() {
        Long count = em.createQuery(
                "SELECT COUNT(g) FROM GameModel g", Long.class
        ).getSingleResult();
        return count > 0;
    }

    public GameModel getGame() {
        return em.createQuery("FROM GameModel", GameModel.class)
                .getResultStream()
                .findFirst()
                .orElse(null);
    }
}

