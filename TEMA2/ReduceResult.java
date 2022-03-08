public class ReduceResult {

    String filename;
    float rang;
    int longestword;
    int occurences;

    public ReduceResult(String filename, float rang, int longestword, int occurences) {
        this.filename = filename;
        this.rang = rang;
        this.longestword = longestword;
        this.occurences = occurences;
    }

    @Override
    public String toString() {
        return "ReduceResult{" +
                "filename='" + filename + '\'' +
                ", rang=" + rang +
                ", longestword=" + longestword +
                ", occurences=" + occurences +
                '}';
    }

}