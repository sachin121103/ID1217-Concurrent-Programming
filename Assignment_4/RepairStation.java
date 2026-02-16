class RepairSimulation {
    private final int maxA, maxB, maxC, v;

    private int currentA = 0;
    private int currentB = 0;
    private int currentC = 0;

    public RepairSimulation(int a, int b, int c, int v){
        this.maxA = a;
        this.maxB = b;
        this.maxC = c;
        this.v = v;
    }

    public synchronized void requestRepair(char type) throws InterruptedException {

    }

    public synchronized void releaseRepair(char type) throws InterruptedException {
        
    }
}

class Vehicle extends Thread {

    
}


public class RepairStation {
    public static void main(String[] args) {
        
    }

    
}