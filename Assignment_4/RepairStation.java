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

    public synchronized void requestRepair(char type, int id) throws InterruptedException {
        while ((currentA+currentB+currentC >= v) || 
        (type=='A' && currentA >= maxA) || 
        (type=='B' && currentB >= maxB) || 
        (type=='C' && currentC >= maxC)) {
            
            System.out.println("Vehicle " + type + id + " is waiting for a spot...");
            wait();
        }

        if (type == 'A') {
            currentA++;
        }

        else if (type == 'B') {
            currentB++;
        }

        else if (type == 'C') {
            currentC++;
        }

        System.out.println("Vehicle " + type + id + " Entered. Status: Vehicle A: " + currentA + ", Vehicle B: " + currentB + ", Vehicle C: " + currentC);
    }

    public synchronized void releaseRepair(char type, int id){
        if (type == 'A') {
            currentA--;
        }

        else if (type == 'B') {
            currentB--;
        }

        else if (type == 'C') {
            currentC--;
        }

        System.out.println("Vehicle " + type + id + " Left. Status: Vehicle A: " + currentA + ", Vehicle B: " + currentB + ", Vehicle C: " + currentC);
        notifyAll();
    }
}

class Vehicle extends Thread {
    private final RepairSimulation station;
    private final char type;
    private final int id;
    
    public Vehicle(RepairSimulation station, char type, int id) {
        this.station = station;
        this.type = type;
        this.id = id;
    }

    @Override
    public void run() {
        try {
            station.requestRepair(type, id);
    
            long repairTime = System.currentTimeMillis() % 1000;
            System.out.println("Vehicle " + type + id + " repairing for " + repairTime + "ms");
    
            Thread.sleep(repairTime); 
    
            station.releaseRepair(type, id);
    
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
    
}


public class RepairStation {
    public static void main(String[] args) {
        RepairSimulation station = new RepairSimulation(2, 2, 2, 3);

        new Vehicle(station, 'A', 1).start();
        new Vehicle(station, 'A', 2).start();
        new Vehicle(station, 'B', 1).start();
        new Vehicle(station, 'B', 2).start();
        new Vehicle(station, 'C', 1).start();
        new Vehicle(station, 'C', 2).start();

    }

    
}