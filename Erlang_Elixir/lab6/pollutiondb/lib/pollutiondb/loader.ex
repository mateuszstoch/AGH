defmodule Pollutiondb.Loader do
  alias Pollutiondb.{Station, Reading}

  def import_from_csv(filepath) do
    filepath
    |> File.read!()
    |> String.trim()
    |> String.split("\n")
    |> Enum.each(&import_line/1)
  end

  def import_line("") do
    :ok
  end

  def import_line(line) do
    case String.split(line, ";") do
      [dateTime_str, type, value_str, _stationId_str, stationName, loc_str] ->
        # Parse location
        [lat_str, lon_str] = String.split(loc_str, ",")
        lat = String.to_float(lat_str)
        lon = String.to_float(lon_str)

        # Parse datetime
        datetime = NaiveDateTime.from_iso8601!(dateTime_str)
        date = NaiveDateTime.to_date(datetime)
        time = NaiveDateTime.to_time(datetime)

        # Parse value
        value = String.to_float(value_str)

        # Check if station exists, otherwise insert
        station =
          case Station.get_by_name(stationName) do
            [] ->
              case Station.add(stationName, lon, lat) do
                {:ok, station} -> station
                {:error, reason} -> raise "Failed to insert station: #{inspect(reason)}"
              end
            [existing | _] ->
              existing
          end

        # Add reading
        case Reading.add(station, date, time, type, value) do
          {:ok, reading} -> {:ok, station, reading}
          {:error, reason} -> {:error, reason}
        end

      _ ->
        :ok
    end
  end
end
