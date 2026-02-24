package fr.epita.assistants.yakamon.utils;

import lombok.AllArgsConstructor;
import lombok.Getter;

@AllArgsConstructor
@Getter
public enum Direction {
    UP(new Point(0, -1)),
    RIGHT(new Point(1, 0)),
    DOWN(new Point(0, 1)),
    LEFT(new Point(-1, 0));

    private final Point point;
}
