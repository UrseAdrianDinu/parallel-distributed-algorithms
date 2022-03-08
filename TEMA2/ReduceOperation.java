import java.util.ArrayList;
import java.util.concurrent.*;

public class ReduceOperation implements ReduceInterface<Integer, Integer> {

    int workers;
    ArrayList<MapResult<Integer, Integer>> mapResults;
    int no_files;
    ArrayList<String> input_files;

    public ReduceOperation(int workers, ArrayList<MapResult<Integer, Integer>> mapResults, int no_files, ArrayList<String> input_files) {
        this.workers = workers;
        this.mapResults = mapResults;
        this.no_files = no_files;
        this.input_files = input_files;
    }

    @Override
    public ArrayList<ReduceResult> reduce(ArrayList<MapResult<Integer, Integer>> mapResults) {

        // Pool pentru task-urile Reduce
        ExecutorService reduce_pool = Executors.newFixedThreadPool(workers);

        // Array pentru rezultatele etapei Reduce
        ArrayList<ReduceResult> reduceResults = new ArrayList<>();

        ArrayList<CompletableFuture<ReduceResult>> futures = new ArrayList<>();

        // Crearea task-urilor Reduce
        for (int i = 0; i < no_files; i++) {
            ArrayList<MapResult<Integer, Integer>> res = new ArrayList<>();
            for (MapResult<Integer, Integer> mapResult : mapResults) {
                if (mapResult.filename.equals(input_files.get(i))) {
                    res.add(mapResult);
                }
            }

            CompletableFuture<ReduceResult> completableFuture = new CompletableFuture<>();
            // Adaugarea task-urile Reduce in pool
            reduce_pool.submit(new ReduceTask(input_files.get(i), res, completableFuture));
            futures.add(completableFuture);
        }

        // Preluarea rezultatelor din obiectele Future
        for (CompletableFuture<ReduceResult> future : futures) {
            ReduceResult result = null;
            try {
                result = future.get();
            } catch (InterruptedException | ExecutionException e) {
                e.printStackTrace();
            }
            reduceResults.add(result);
        }

        reduce_pool.shutdown();

        return reduceResults;
    }
}
