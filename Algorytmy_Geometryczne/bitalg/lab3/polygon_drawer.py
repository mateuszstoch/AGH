import matplotlib.pyplot as plt
from matplotlib.lines import Line2D
import ast


class PolygonEditor:
    def __init__(self):
        self.fig, self.ax = plt.subplots(figsize=(8, 8))
        self.points = []

        self.line, = self.ax.plot([], [], 'o-', color='blue', markersize=5)
        self.preview_line, = self.ax.plot(
            [], [], '--', color='grey', linewidth=0.5)

        self.ax.set_title(
            "Edytor Wielokątów\nLPM: Dodaj | PPM: Zamknij | 's': Zapisz | 'l': Wczytaj | 'z': Cofnij | 'r': Reset")
        self.ax.set_xlim(0, 10)
        self.ax.set_ylim(0, 10)
        self.ax.grid(True)

        self.cid_click = self.fig.canvas.mpl_connect(
            'button_press_event', self.on_click)
        self.cid_key = self.fig.canvas.mpl_connect(
            'key_press_event', self.on_key)
        self.cid_move = self.fig.canvas.mpl_connect(
            'motion_notify_event', self.on_move)

        print("=== INSTRUKCJA ===")
        print("Kliknij na wykres, aby dodać punkty.")
        print("[LPM] - Dodaj punkt")
        print("[PPM] - Zamknij wielokąt")
        print("[s]   - Zapisz do pliku 'polygon_data.txt'")
        print("[l]   - Wczytaj z pliku 'polygon_data.txt'")
        print("[z]   - Cofnij ostatni punkt")
        print("[r]   - Resetuj (wyczyść wszystko)")

        plt.show()

    def update_plot(self):
        if not self.points:
            self.line.set_data([], [])
            self.fig.canvas.draw()
            return

        xs, ys = zip(*self.points)
        self.line.set_data(xs, ys)
        self.fig.canvas.draw()

    def on_click(self, event):
        if event.inaxes != self.ax or self.fig.canvas.toolbar.mode != '':
            return

        if event.button == 1:
            x, y = round(event.xdata, 2), round(event.ydata, 2)
            self.points.append((x, y))
            self.update_plot()
            print(f"Dodano punkt: ({x}, {y})")
        elif event.button == 3:
            if len(self.points) > 2:
                xs, ys = zip(*self.points)
                xs = list(xs) + [xs[0]]
                ys = list(ys) + [ys[0]]
                self.line.set_data(xs, ys)
                self.fig.canvas.draw()
                print("Wielokąt zamknięty (wizualnie).")

    def on_move(self, event):
        if event.inaxes != self.ax or not self.points:
            self.preview_line.set_data([], [])
            self.fig.canvas.draw()
            return

        last_p = self.points[-1]
        self.preview_line.set_data([last_p[0], event.xdata], [
                                   last_p[1], event.ydata])
        self.fig.canvas.draw()

    def on_key(self, event):
        if event.key == 's':
            with open("polygon_data.txt", "w") as f:
                f.write(str(self.points))
            print(f"\nZapisano do pliku polygon_data.txt:\n{self.points}")
            self.ax.set_title("Zapisano!")
            self.fig.canvas.draw()

        elif event.key == 'l':
            try:
                with open("polygon_data.txt", "r") as f:
                    content = f.read()
                    loaded_points = ast.literal_eval(content)
                    if isinstance(loaded_points, list):
                        self.points = loaded_points
                        self.update_plot()
                        print(f"\nWczytano:\n{self.points}")
                        self.ax.set_title("Wczytano z pliku!")
                        self.fig.canvas.draw()
            except FileNotFoundError:
                print("Nie znaleziono pliku polygon_data.txt")
            except Exception as e:
                print(f"Błąd podczas wczytywania: {e}")

        elif event.key == 'z':
            if self.points:
                removed = self.points.pop()
                print(f"Usunięto: {removed}")
                self.update_plot()

        elif event.key == 'r':
            self.points = []
            self.update_plot()
            print("Wyczyszczono.")
            self.ax.set_title("Edytor Wielokątów")
            self.fig.canvas.draw()


if __name__ == "__main__":
    editor = PolygonEditor()
