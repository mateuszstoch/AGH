--zadanie 1
select o.EmployeeID ,count(*) from Orders o GROUP BY o.EmployeeID 

--zadanie 2
select o.ShipVia, round(sum(o.Freight),2) from Orders o Group by o.ShipVia 

--zadanie 3
select o.ShipVia, round(sum(o.Freight),2) from Orders o where year(o.ShippedDate) BETWEEN 1996 and 1997 Group by o.ShipVia 
