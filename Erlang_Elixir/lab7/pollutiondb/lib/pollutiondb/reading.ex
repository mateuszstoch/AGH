defmodule Pollutiondb.Reading do
  use Ecto.Schema
  require Ecto.Query

  schema "readings" do
    field(:date, :date)
    field(:time, :time)
    field(:type, :string)
    field(:value, :float)

    belongs_to :station, Pollutiondb.Station
  end

  def add_now(station_id, type, value) when is_integer(station_id) do
    station = Pollutiondb.Station.get_by_id(station_id)
    add_now(station, type, value)
  end

  def add_now(%Pollutiondb.Station{} = station, type, value) do
    Ecto.build_assoc(station, :readings, %{
      date: Date.utc_today(),
      time: Time.truncate(Time.utc_now(), :second),
      type: type,
      value: value
    })
    |> Pollutiondb.Repo.insert()
  end

  def add(station_id, date, time, type, value) when is_integer(station_id) do
    station = Pollutiondb.Station.get_by_id(station_id)
    add(station, date, time, type, value)
  end

  def add(%Pollutiondb.Station{} = station, date, time, type, value) do
    Ecto.build_assoc(station, :readings, %{
      date: date,
      time: time,
      type: type,
      value: value
    })
    |> Pollutiondb.Repo.insert()
  end

  def find_by_date(date_str) when is_binary(date_str) do
    find_by_date(Date.from_iso8601!(date_str))
  end

  def find_by_date(%Date{} = date) do
    Pollutiondb.Reading
    |> Ecto.Query.where(date: ^date)
    |> Ecto.Query.preload(:station)
    |> Pollutiondb.Repo.all()
  end
end
