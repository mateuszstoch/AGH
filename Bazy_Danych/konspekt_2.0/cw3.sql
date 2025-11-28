--zadanie 1
select od.OrderID,count(*) from [Order Details] od 
group by od.OrderID having count(*) > 5

--zadanie 2
select o.CustomerID, count(*) from Orders o 
where year(o.ShippedDate) = 1998
group by o.CustomerID having count(*) > 8
order by sum(o.Freight) desc