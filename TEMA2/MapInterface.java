import java.util.ArrayList;

// Interfata pentru operatia Map
public interface MapInterface<K, V> {
    ArrayList<MapResult<K, V>> map();
}
