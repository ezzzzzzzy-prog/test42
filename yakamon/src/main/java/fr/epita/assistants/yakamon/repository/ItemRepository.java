package fr.epita.assistants.yakamon.repository;

import fr.epita.assistants.yakamon.data.model.ItemModel;
import jakarta.enterprise.context.ApplicationScoped;
import jakarta.inject.Inject;
import jakarta.persistence.EntityManager;

import java.util.List;

@ApplicationScoped
public class ItemRepository {

    @Inject
    EntityManager em;

    public List<ItemModel> findAll() {
        return em.createQuery("FROM ItemModel", ItemModel.class).getResultList();
    }

    public void deleteAll() {
        em.createQuery("DELETE FROM ItemModel").executeUpdate();
    }

    public void persist(ItemModel yakaballs) {
    }
}
