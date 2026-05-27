defmodule Pollutiondb.Station do
  use Ecto.Schema
  require Ecto.Query

  schema "stations" do
    field(:name, :string)
    field(:lon, :float)
    field(:lat, :float)

    has_many(:readings, Pollutiondb.Reading)
  end

  def add(name, lon, lat) do
    %Pollutiondb.Station{name: name, lon: lon, lat: lat}
    |> Pollutiondb.Repo.insert()
  end

  def add(stations) do
    stations |> Enum.map(fn station -> Pollutiondb.Repo.insert(station) end)
  end

  def get_all() do
    Pollutiondb.Repo.all(Pollutiondb.Station)
  end

  def get_by_id(id) do
    Pollutiondb.Repo.get(Pollutiondb.Station, id)
  end

  def get_by_name(name) do
    Pollutiondb.Repo.all(Ecto.Query.where(Pollutiondb.Station, name: ^name))
  end

  def get_by_location(lon, lat) do
    Ecto.Query.from(s in Pollutiondb.Station,
      where: s.lon == ^lon,
      where: s.lat == ^lat
    )
    |> Pollutiondb.Repo.all()
  end

  def remove(station) do
    Pollutiondb.Repo.delete(station)
  end
end
