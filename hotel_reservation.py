import sys
import random

class Room:
    def __init__(self, floor, index):
        self.floor = floor
        self.index = index  # Horizontal distance from lift
        self.is_booked = False
        
        # Room Number Logic 
        # Floor 1-9: 101, 102... Floor 10: 1001...
        if floor < 10:
            self.number = (floor * 100) + (index + 1)
        else:
            self.number = 1000 + (index + 1)

    def __repr__(self):
        return str(self.number)

class HotelSystem:
    def __init__(self):
        self.rooms = []
        self.last_booked_ids = []  # For visualization highlighting
        self._init_hotel()

    def _init_hotel(self):
        """Initialize 97 rooms across 10 floors"""
        self.rooms = []
        for f in range(1, 11):
            # Floor 10 has 7 rooms, others have 10 
            count = 7 if f == 10 else 10
            for i in range(count):
                self.rooms.append(Room(f, i))

    def print_grid(self):
        """Visualizes the hotel grid in the console"""
        print("\n" + "="*40)
        print(" HOTEL VISUALIZATION ")
        print(" [ ]=Avail  [X]=Booked  [*]=Just Booked")
        print("="*40 + "\n")

        # Print from Top Floor (10) down to 1
        for f in range(10, 0, -1):
            floor_rooms = [r for r in self.rooms if r.floor == f]
            floor_rooms.sort(key=lambda r: r.index)
            
            row_str = f"Floor {f:2} | "
            
            for r in floor_rooms:
                if r.number in self.last_booked_ids:
                    row_str += "[*] " # Highlight recent booking
                elif r.is_booked:
                    row_str += "[X] " # Already booked
                else:
                    row_str += "[ ] " # Available
            
            print(row_str)
        
        print("         ^ LIFT/STAIRS ^\n")

    def reset(self):
        self._init_hotel()
        self.last_booked_ids = []
        print("\n>>> System Reset. All rooms available.")

    def generate_random(self):
        """Randomly books ~30% of rooms to simulate a real scenario"""
        for r in self.rooms:
            r.is_booked = (random.random() < 0.3)
        self.last_booked_ids = []
        print("\n>>> Random occupancy generated.")
        self.print_grid()

    def book_rooms(self, n):
        """
        Main Algorithm [cite: 29-33]
        1. Try to book 'n' rooms on the same floor (minimize horizontal travel).
        2. If not possible, book across floors (minimize total travel time).
        """
        if not (1 <= n <= 5):
            print("\n>>> Error: You can only book 1 to 5 rooms.")
            return

        available = [r for r in self.rooms if not r.is_booked]
        
        if len(available) < n:
            print("\n>>> Error: Not enough rooms available.")
            return

        best_set = []
        min_cost = float('inf')
        found_same_floor = False

        # --- PRIORITY 1: SAME FLOOR OPTIMIZATION ---
        # Group available rooms by floor
        floors = {}
        for r in available:
            if r.floor not in floors:
                floors[r.floor] = []
            floors[r.floor].append(r)

        for f, floor_rooms in floors.items():
            if len(floor_rooms) >= n:
                # Sort by index to find contiguous blocks
                floor_rooms.sort(key=lambda r: r.index)

                # Sliding window of size 'n'
                for i in range(len(floor_rooms) - n + 1):
                    subset = floor_rooms[i : i+n]
                    
                    # Cost = Horizontal Distance (Last Index - First Index) * 1 min
                    cost = subset[-1].index - subset[0].index
                    
                    if cost < min_cost:
                        min_cost = cost
                        best_set = subset
                        found_same_floor = True

        # --- PRIORITY 2: CROSS-FLOOR OPTIMIZATION ---

        if not found_same_floor:
            print("\n>>> Info: Optimizing across floors (Minimizing Travel Time)...")
            
            # Heuristic: Linearize the 2D grid by "Proximity Score"
            # Score = (Floor * 2) + Index
            # This logic groups rooms physically close to the lift together
            sorted_available = sorted(available, key=lambda r: (r.floor * 2) + r.index)
            
            min_cost = float('inf') # Reset for global search

            # Sliding window over the proximity-sorted list
            for i in range(len(sorted_available) - n + 1):
                subset = sorted_available[i : i+n]
                
                # Calculate "Bounding Box" cost
                floors = [r.floor for r in subset]
                indexes = [r.index for r in subset]
                
                # Vertical Cost: 2 mins per floor difference [cite: 28]
                v_cost = (max(floors) - min(floors)) * 2
                
                # Horizontal Cost: 1 min per room difference [cite: 27]
                h_cost = (max(indexes) - min(indexes)) * 1
                
                total_cost = v_cost + h_cost
                
                if total_cost < min_cost:
                    min_cost = total_cost
                    best_set = subset

        # --- APPLY BOOKING ---
        if best_set:
            self.last_booked_ids = []
            room_nums = []
            for r in best_set:
                r.is_booked = True
                self.last_booked_ids.append(r.number)
                room_nums.append(str(r.number))
            
            print(f"\n>>> Success! Booked: {', '.join(room_nums)}")
            print(f">>> Travel Cost Metric: {min_cost}")
            self.print_grid()
        else:
            print("\n>>> Error: Could not find a suitable room arrangement.")

# --- MAIN EXECUTION LOOP ---

def main():
    system = HotelSystem()
    system.print_grid()

    while True:
        print("\n--- MENU ---")
        print("1. Book Rooms")
        print("2. Generate Random Occupancy")
        print("3. Reset System")
        print("4. Show Grid")
        print("0. Exit")
        
        choice = input("Enter choice: ")

        if choice == '0':
            print("Exiting...")
            break
        elif choice == '1':
            try:
                n = int(input("Enter number of rooms (1-5): "))
                system.book_rooms(n)
            except ValueError:
                print("Invalid input. Please enter a number.")
        elif choice == '2':
            system.generate_random()
        elif choice == '3':
            system.reset()
            system.print_grid()
        elif choice == '4':
            system.print_grid()
        else:
            print("Invalid command.")

if __name__ == "__main__":
    main()
