--zadanie 1
select max(od.UnitPrice) as maxprice from [Order Details] od 
group by od.OrderID order by maxprice desc

--zadanie 2
select max(od.UnitPrice),min(od.UnitPrice ) from [Order Details] od 
group by od.OrderID 

--zadanie 3
select count(o.OrderID) from Orders o group by o.ShipVia 

--zadanie 4
select top 1 o.ShipVia, count(o.orderID) as number from Orders o 
where year(o.ShippedDate) = 1997
group by o.ShipVia order by number desc