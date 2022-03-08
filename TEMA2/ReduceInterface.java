import java.util.ArrayList;

// Interfata pentru operatia Reduce
public interface ReduceInterface<K, V> {
    ArrayList<ReduceResult> reduce(ArrayList<MapResult<K, V>> mapResults);
}
